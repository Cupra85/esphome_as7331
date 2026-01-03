#include "as7331.h"
#include "esphome/core/log.h"

namespace esphome {
namespace as7331 {

static const char *TAG = "as7331";

void AS7331Component::setup() {
  ESP_LOGI(TAG, "AS7331 setup complete");
}

void AS7331Component::update() {
  if (!start_measurement_()) {
    ESP_LOGW(TAG, "Measurement start failed");
    return;
  }

  if (!wait_for_data_()) {
    ESP_LOGW(TAG, "No data ready");
    return;
  }

  uint16_t m1 = 0, m2 = 0, m3 = 0;
  if (!read_u16_(REG_MRES1, m1) ||
      !read_u16_(REG_MRES2, m2) ||
      !read_u16_(REG_MRES3, m3)) {
    ESP_LOGW(TAG, "Failed to read result registers");
    return;
  }

  if (uva_) uva_->publish_state(m1);
  if (uvb_) uvb_->publish_state(m2);
  if (uvc_) uvc_->publish_state(m3);

  if (uva_irr_) uva_irr_->publish_state(m1 * uva_mult_);
  if (uvb_irr_) uvb_irr_->publish_state(m2 * uvb_mult_);
  if (uvc_irr_) uvc_irr_->publish_state(m3 * uvc_mult_);
}

bool AS7331Component::start_measurement_() {
  uint8_t osr = 0;
  if (!this->read_byte(REG_OSR, &osr)) return false;

  osr = (osr & ~OSR_DOS_MASK) | DOS_MEAS;
  osr |= OSR_SS;

  return this->write_byte(REG_OSR, osr);
}

bool AS7331Component::wait_for_data_() {
  for (int i = 0; i < 50; i++) {
    uint16_t test = 0;
    if (!read_u16_(REG_MRES1, test)) return false;
    if (test != 0) return true;
    delay(10);
  }
  return false;
}

bool AS7331Component::read_u16_(uint8_t reg, uint16_t &out) {
  uint8_t data[2];
  if (!this->read_bytes(reg, data, 2)) return false;
  out = (uint16_t(data[1]) << 8) | data[0];
  return true;
}

}  // namespace as7331
}  // namespace esphome
