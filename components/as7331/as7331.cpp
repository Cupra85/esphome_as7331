#include "as7331.h"
#include "esphome/core/log.h"

namespace esphome {
namespace as7331 {

static const char *TAG = "as7331";

void AS7331Component::setup() {
  write_cfg_();
}

void AS7331Component::update() {
  if (!start_measurement_()) {
    ESP_LOGW(TAG, "Measurement start failed");
    return;
  }

  if (!wait_for_data_()) {
    ESP_LOGD(TAG, "No new data, re-publishing last values");

    if (uva_) uva_->publish_state(last_uva_);
    if (uvb_) uvb_->publish_state(last_uvb_);
    if (uvc_) uvc_->publish_state(last_uvc_);

    if (uva_irr_) uva_irr_->publish_state(last_uva_ * uva_mult_);
    if (uvb_irr_) uvb_irr_->publish_state(last_uvb_ * uvb_mult_);
    if (uvc_irr_) uvc_irr_->publish_state(last_uvc_ * uvc_mult_);
    return;
  }

  uint16_t m1 = 0, m2 = 0, m3 = 0;
  if (!read_u16_(REG_MRES1, m1) ||
      !read_u16_(REG_MRES2, m2) ||
      !read_u16_(REG_MRES3, m3)) {
    ESP_LOGW(TAG, "Failed to read MRES registers");
    return;
  }

  last_uva_ = m1;
  last_uvb_ = m2;
  last_uvc_ = m3;

  if (uva_) uva_->publish_state(m1);
  if (uvb_) uvb_->publish_state(m2);
  if (uvc_) uvc_->publish_state(m3);

  if (uva_irr_) uva_irr_->publish_state(m1 * uva_mult_);
  if (uvb_irr_) uvb_irr_->publish_state(m2 * uvb_mult_);
  if (uvc_irr_) uvc_irr_->publish_state(m3 * uvc_mult_);
}

bool AS7331Component::write_cfg_() {
  uint8_t creg1 = (gain_ << 4) | (conv_time_ & 0x0F);
  uint8_t creg2 = (divider_ & 0x07) | (en_div_ ? 0x08 : 0);
  uint8_t creg3 = (cclk_ & 0x03) | ((meas_mode_ & 0x03) << 6);

  return write_byte(REG_CREG1, creg1) &&
         write_byte(REG_CREG2, creg2) &&
         write_byte(REG_CREG3, creg3);
}

bool AS7331Component::start_measurement_() {
  return write_byte(REG_OSR, DOS_MEAS | OSR_SS);
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
  if (!read_bytes(reg, data, 2)) return false;
  out = (uint16_t(data[1]) << 8) | data[0];
  return true;
}

}  // namespace as7331
}  // namespace esphome
