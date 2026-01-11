#include "as7331.h"
#include "esphome/core/log.h"

namespace esphome {
namespace as7331 {

static const char *const TAG = "as7331";

// NOTE: Register addresses below are placeholders to keep the project complete.
// Replace with datasheet-accurate addresses/bitfields for production calibration.

static const uint8_t REG_CONFIG = 0x00;
static const uint8_t REG_DATA_START = 0x10;  // 6 bytes: UVA, UVB, UVC

bool AS7331Component::init_device_() {
  // Minimal init: write something non-destructive.
  // Replace with real config: gain, integration time, measurement mode, etc.
  uint8_t buf[2] = {REG_CONFIG, 0x01};
  if (!this->write(buf, 2)) {
    ESP_LOGE(TAG, "I2C write init failed");
    return false;
  }
  return true;
}

bool AS7331Component::read_raw_(uint16_t &uva, uint16_t &uvb, uint16_t &uvc) {
  uint8_t reg = REG_DATA_START;
  uint8_t data[6]{0};

  if (!this->write_read(&reg, 1, data, sizeof(data))) {
    ESP_LOGE(TAG, "I2C read failed");
    return false;
  }

  uva = (uint16_t(data[0]) << 8) | data[1];
  uvb = (uint16_t(data[2]) << 8) | data[3];
  uvc = (uint16_t(data[4]) << 8) | data[5];
  return true;
}

void AS7331Component::setup() {
  ESP_LOGI(TAG, "Setting up AS7331");
  if (!init_device_()) {
    ESP_LOGW(TAG, "AS7331 init failed; will retry on next update()");
  }
}

void AS7331Component::update() {
  uint16_t raw_uva = 0, raw_uvb = 0, raw_uvc = 0;

  if (!read_raw_(raw_uva, raw_uvb, raw_uvc)) {
    // Try to re-init once if reading fails.
    init_device_();
    return;
  }

  // Scale placeholders: replace with datasheet scaling / calibration.
  const float uva = raw_uva * 0.001f;
  const float uvb = raw_uvb * 0.001f;
  const float uvc = raw_uvc * 0.001f;

  if (uva_) uva_->publish_state(uva);
  if (uvb_) uvb_->publish_state(uvb);
  if (uvc_) uvc_->publish_state(uvc);

  // Simple UV Index approximation placeholder. Replace with standard weighting if desired.
  // Keep it deterministic and monotonic with channel intensities.
  const float uvi = (uva * 0.0029f) + (uvb * 0.058f) + (uvc * 0.0020f);
  if (uvi_) uvi_->publish_state(uvi);
}

}  // namespace as7331
}  // namespace esphome
