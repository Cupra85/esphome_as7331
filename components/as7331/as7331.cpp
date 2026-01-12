#include "as7331.h"
#include "esphome/core/log.h"

namespace esphome {
namespace as7331 {

static const char *TAG = "as7331";

/* =========================================================
 * AS7331 REGISTER MAP (SparkFun / Datenblatt korrekt)
 * ========================================================= */
static const uint8_t REG_OSR    = 0x00;

static const uint8_t REG_CREG1  = 0x06;  // Gain + Integration
static const uint8_t REG_CREG2  = 0x07;  // Divider
static const uint8_t REG_CREG3  = 0x08;  // Measurement Mode (CONT)

static const uint8_t REG_MRES1  = 0x10;  // UVA MSB
static const uint8_t REG_MRES2  = 0x12;  // UVB MSB
static const uint8_t REG_MRES3  = 0x14;  // UVC MSB

/* =========================================================
 * OSR BITS
 * ========================================================= */
static const uint8_t DOS_MEAS = 0x40;  // Device Operating State: Measurement
static const uint8_t OSR_SS   = 0x20;  // Start Measurement Trigger

/* =========================================================
 * CONFIGURATION
 * ========================================================= */
void AS7331Component::configure_() {
  ESP_LOGI(TAG, "Configuring AS7331");

  // CREG1: GAIN[7:4] | INTEGRATION_TIME[3:0]
  uint8_t creg1 = ((gain_ & 0x0F) << 4) | (integration_time_ & 0x0F);
  write_byte(REG_CREG1, creg1);

  // CREG2: Divider = 0 (default, SparkFun)
  write_byte(REG_CREG2, 0x00);

  // CREG3: Measurement Mode = CONT (0)
  write_byte(REG_CREG3, 0x00);

  ESP_LOGI(TAG, "AS7331 configured: CONT mode enabled");
}

/* =========================================================
 * MEASUREMENT CONTROL
 * ========================================================= */
void AS7331Component::start_measurement_() {
  // Trigger a measurement (required even in CONT mode)
  write_byte(REG_OSR, DOS_MEAS | OSR_SS);
}

/* =========================================================
 * ESPHOME LIFECYCLE
 * ========================================================= */
void AS7331Component::setup() {
  ESP_LOGI(TAG, "Setting up AS7331");

  configure_();

  // Initial trigger so first update has data
  start_measurement_();
}

/* =========================================================
 * UPDATE = DAUERMESSUNG (CONT)
 * ========================================================= */
void AS7331Component::update() {
  // 🔑 CONT mode requires a trigger for EVERY measurement
  write_byte(REG_OSR, DOS_MEAS | OSR_SS);

  uint8_t buf[2];
  uint16_t uva;
  uint16_t uvb;
  uint16_t uvc;

  // UVA
  if (!read_bytes(REG_MRES1, buf, 2)) {
    ESP_LOGW(TAG, "Failed to read UVA");
    return;
  }
  uva = (uint16_t(buf[0]) << 8) | buf[1];

  // UVB
  if (!read_bytes(REG_MRES2, buf, 2)) {
    ESP_LOGW(TAG, "Failed to read UVB");
    return;
  }
  uvb = (uint16_t(buf[0]) << 8) | buf[1];

  // UVC
  if (!read_bytes(REG_MRES3, buf, 2)) {
    ESP_LOGW(TAG, "Failed to read UVC");
    return;
  }
  uvc = (uint16_t(buf[0]) << 8) | buf[1];

  // Publish values
  if (uva_) uva_->publish_state(uva);
  if (uvb_) uvb_->publish_state(uvb);
  if (uvc_) uvc_->publish_state(uvc);
}

}  // namespace as7331
}  // namespace esphome
