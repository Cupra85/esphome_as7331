#include "as7331.h"
#include "esphome/core/log.h"

namespace esphome {
namespace as7331 {

static const char *TAG = "as7331";

/* === REGISTER MAP (SparkFun) === */
static const uint8_t REG_OSR    = 0x00;
static const uint8_t REG_CREG1  = 0x06;
static const uint8_t REG_CREG2  = 0x07;
static const uint8_t REG_CREG3  = 0x08;

static const uint8_t REG_MRES1  = 0x10;
static const uint8_t REG_MRES2  = 0x12;
static const uint8_t REG_MRES3  = 0x14;

static const uint8_t DOS_MEAS = 0x40;
static const uint8_t OSR_SS   = 0x20;

/* === CONFIG === */

void AS7331Component::configure_() {
  uint8_t creg1 = (gain_ << 4) | (integration_time_ & 0x0F);
  write_byte(REG_CREG1, creg1);

  write_byte(REG_CREG2, 0x00);  // divider
  write_byte(REG_CREG3, 0x00);  // CONT mode
}

/* === MEASUREMENT === */

void AS7331Component::start_measurement_() {
  write_byte(REG_OSR, DOS_MEAS | OSR_SS);
  ESP_LOGI(TAG, "AS7331 measurement started (CONT)");
}

void AS7331Component::setup() {
  ESP_LOGI(TAG, "Initializing AS7331 (SparkFun-based)");
  configure_();
  start_measurement_();
}

void AS7331Component::update() {
  uint8_t buf[2];
  uint16_t uva, uvb, uvc;

  if (!read_bytes(REG_MRES1, buf, 2)) {
    ESP_LOGW(TAG, "Failed to read UVA");
    return;
  }
  uva = (uint16_t(buf[0]) << 8) | buf[1];

  if (!read_bytes(REG_MRES2, buf, 2)) {
    ESP_LOGW(TAG, "Failed to read UVB");
    return;
  }
  uvb = (uint16_t(buf[0]) << 8) | buf[1];

  if (!read_bytes(REG_MRES3, buf, 2)) {
    ESP_LOGW(TAG, "Failed to read UVC");
    return;
  }
  uvc = (uint16_t(buf[0]) << 8) | buf[1];

  if (uva_) uva_->publish_state(uva);
  if (uvb_) uvb_->publish_state(uvb);
  if (uvc_) uvc_->publish_state(uvc);
}


}  // namespace as7331
}  // namespace esphome
