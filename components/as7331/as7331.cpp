#include "as7331.h"
#include "esphome/core/log.h"

namespace esphome {
namespace as7331 {

static const char *TAG = "as7331";

/* =========================================================
 * REGISTER MAP (SparkFun / Datenblatt)
 * ========================================================= */
static const uint8_t REG_OSR    = 0x00;
static const uint8_t REG_CREG1  = 0x06;
static const uint8_t REG_CREG2  = 0x07;
static const uint8_t REG_CREG3  = 0x08;

static const uint8_t REG_MRES1  = 0x10;
static const uint8_t REG_MRES2  = 0x12;
static const uint8_t REG_MRES3  = 0x14;

/* =========================================================
 * OSR BITS
 * ========================================================= */
static const uint8_t DOS_MEAS = 0x40;
static const uint8_t OSR_SS   = 0x20;

/* =========================================================
 * CONFIGURATION
 * ========================================================= */
void AS7331Component::configure_() {
  ESP_LOGI(TAG, "Configuring AS7331");

  // CREG1: GAIN[7:4] | INT_TIME[3:0]
  uint8_t creg1 = ((gain_ & 0x0F) << 4) | (integration_time_ & 0x0F);
  write_byte(REG_CREG1, creg1);

  // Divider default
  write_byte(REG_CREG2, 0x00);

  // CONT mode
  write_byte(REG_CREG3, 0x00);

  ESP_LOGI(TAG, "AS7331 configured (CONT mode)");
}

/* =========================================================
 * SETUP
 * ========================================================= */
void AS7331Component::setup() {
  ESP_LOGI(TAG, "Setting up AS7331");

  configure_();

  // Initial trigger
  write_byte(REG_OSR, DOS_MEAS | OSR_SS);
}

/* =========================================================
 * UPDATE = CONT + CORRECT TIMING
 * ========================================================= */
void AS7331Component::update() {
  // 1️⃣ Trigger new measurement
  write_byte(REG_OSR, DOS_MEAS | OSR_SS);

  // 2️⃣ Wait for integration time
  // integration_time_ is log2(ms)
  uint16_t integration_ms = 1 << integration_time_;
  delay(integration_ms + 5);  // safety margin

  // 3️⃣ Read result registers
  uint8_t buf[2];
  uint16_t uva, uvb, uvc;

  if (!read_bytes(REG_MRES1, buf, 2)) return;
  uva = (uint16_t(buf[0]) << 8) | buf[1];

  if (!read_bytes(REG_MRES2, buf, 2)) return;
  uvb = (uint16_t(buf[0]) << 8) | buf[1];

  if (!read_bytes(REG_MRES3, buf, 2)) return;
  uvc = (uint16_t(buf[0]) << 8) | buf[1];

  // 4️⃣ Publish
  if (uva_) uva_->publish_state(uva);
  if (uvb_) uvb_->publish_state(uvb);
  if (uvc_) uvc_->publish_state(uvc);
}

}  // namespace as7331
}  // namespace esphome
