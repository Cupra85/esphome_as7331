#include "as7331.h"
#include "esphome/core/log.h"

namespace esphome {
namespace as7331 {

static const char *TAG = "as7331";

/* Register addresses (lower 4 bits relevant, see datasheet) */
static const uint8_t REG_OSR   = 0x00;
static const uint8_t REG_AGEN  = 0x02;  // read-only in config-state
static const uint8_t REG_CREG1 = 0x06;
static const uint8_t REG_CREG2 = 0x07;
static const uint8_t REG_CREG3 = 0x08;
static const uint8_t REG_BREAK = 0x09;

/* Output result bank (measurement state) */
static const uint8_t REG_MRES1 = 0x02;  // A
// REG_MRES2 = 0x03
// REG_MRES3 = 0x04

/* OSR bits */
static const uint8_t OSR_SS = 0x80;  // bit7
static const uint8_t OSR_PD = 0x40;  // bit6
// DOS bits are 2:0
static const uint8_t DOS_CONFIG = 0x02;  // 010b
static const uint8_t DOS_MEAS   = 0x03;  // 011b

/* CREG3 MMODE bits 7:6 */
static const uint8_t MMODE_CONT = 0x00;  // 00b
// CCLK bits 1:0: 00 = 1.024 MHz

bool AS7331Component::write_reg_(uint8_t reg, uint8_t value) {
  return this->write_byte(reg, value);
}

void AS7331Component::dump_config() {
  ESP_LOGCONFIG(TAG, "AS7331:");
  LOG_I2C_DEVICE(this);
  ESP_LOGCONFIG(TAG, "  gain: %u (0..15 -> CREG1[7:4])", gain_);
  ESP_LOGCONFIG(TAG, "  int_time: %u (0..15 -> CREG1[3:0])", int_time_);
  LOG_UPDATE_INTERVAL(this);

  if (uva_ != nullptr) LOG_SENSOR("  ", "UVA Raw", uva_);
  if (uvb_ != nullptr) LOG_SENSOR("  ", "UVB Raw", uvb_);
  if (uvc_ != nullptr) LOG_SENSOR("  ", "UVC Raw", uvc_);
}

void AS7331Component::setup() {
  ESP_LOGI(TAG, "Initializing AS7331 (config -> cont -> start)");

  // 1) Ensure we are in CONFIG state and powered (PD=0, DOS=010)
  // OSR: SS=0, PD=0, DOS=010 => 0x02
  if (!write_reg_(REG_OSR, DOS_CONFIG)) {
    ESP_LOGE(TAG, "Failed to write OSR (enter CONFIG)");
    return;
  }
  delay(2);  // allow TSTARTPD margin (typ. 1.2ms) :contentReference[oaicite:9]{index=9}

  // 2) Program CREG1 (GAIN|TIME)
  uint8_t creg1 = uint8_t((gain_ & 0x0F) << 4) | uint8_t(int_time_ & 0x0F);
  if (!write_reg_(REG_CREG1, creg1)) {
    ESP_LOGE(TAG, "Failed to write CREG1");
    return;
  }

  // 3) Program CREG2
  // Keep it simple: divider OFF, EN_TM default doesn't matter unless SYND
  // Datasheet default EN_TM=1 but only relevant in SYND; keep 0x40 to match default behavior. :contentReference[oaicite:10]{index=10}
  uint8_t creg2 = 0x40;  // EN_TM=1, EN_DIV=0, DIV=0
  if (!write_reg_(REG_CREG2, creg2)) {
    ESP_LOGE(TAG, "Failed to write CREG2");
    return;
  }

  // 4) Program CREG3: CONT mode, SB=0, RDYOD=0, CCLK=00 (1.024MHz)
  // MMODE_CONT=00 in bits 7:6 => 0x00
  uint8_t creg3 = MMODE_CONT | 0x00;
  if (!write_reg_(REG_CREG3, creg3)) {
    ESP_LOGE(TAG, "Failed to write CREG3");
    return;
  }

  // 5) BREAK time (pause between conversions) – recommended to avoid I2C overlap; set modest default.
  // BREAK in steps of 8µs; 0x19 default -> 0x19*8µs = 200µs :contentReference[oaicite:11]{index=11}
  write_reg_(REG_BREAK, 0x19);

  // 6) Switch to MEASUREMENT state without starting yet: OSR = 0x03 (SS=0, PD=0, DOS=011)
  if (!write_reg_(REG_OSR, DOS_MEAS)) {
    ESP_LOGE(TAG, "Failed to write OSR (enter MEAS)");
    return;
  }
  delay(2);

  // 7) Start continuous measurement: OSR = 0x83 (SS=1, PD=0, DOS=011) :contentReference[oaicite:12]{index=12}
  if (!write_reg_(REG_OSR, uint8_t(OSR_SS | DOS_MEAS))) {
    ESP_LOGE(TAG, "Failed to start CONT measurement (OSR=0x83)");
    return;
  }

  ESP_LOGI(TAG, "AS7331 started in CONT mode (OSR=0x83).");
}

bool AS7331Component::read_results_(uint16_t &uva, uint16_t &uvb, uint16_t &uvc) {
  // Read 6 bytes starting at MRES1. In measurement state, each result register is 16-bit, LSB first. :contentReference[oaicite:13]{index=13}
  uint8_t buf[6]{0};

  if (!this->read_bytes(REG_MRES1, buf, sizeof(buf)))
    return false;

  // LSB first
  uva = uint16_t(buf[1] << 8) | buf[0];
  uvb = uint16_t(buf[3] << 8) | buf[2];
  uvc = uint16_t(buf[5] << 8) | buf[4];

  return true;
}

void AS7331Component::update() {
  uint16_t uva = 0, uvb = 0, uvc = 0;

  if (!read_results_(uva, uvb, uvc)) {
    ESP_LOGW(TAG, "Failed to read MRES1..3");
    return;
  }

  if (uva_ != nullptr) uva_->publish_state(uva);
  if (uvb_ != nullptr) uvb_->publish_state(uvb);
  if (uvc_ != nullptr) uvc_->publish_state(uvc);

  ESP_LOGD(TAG, "MRES: UVA=%u UVB=%u UVC=%u", uva, uvb, uvc);
}

}  // namespace as7331
}  // namespace esphome
