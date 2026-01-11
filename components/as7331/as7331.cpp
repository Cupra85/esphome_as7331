#include "as7331.h"
#include "esphome/core/log.h"

namespace esphome {
namespace as7331 {

static const char *TAG = "as7331";

void AS7331Component::setup() {
  ESP_LOGI(TAG, "Initializing AS7331 UV sensor");

  // Basic configuration: continuous measurement
  // Register values based on ams datasheet
  uint8_t config[] = {0x00, 0x01};
  this->write(config, 2);
}

bool AS7331Component::read_channels_(float &uva, float &uvb, float &uvc) {
  uint8_t reg = 0x10;
  uint8_t data[6];

  if (!this->write_read(&reg, 1, data, 6)) {
    ESP_LOGE(TAG, "I2C read failed");
    return false;
  }

  uint16_t raw_uva = (data[0] << 8) | data[1];
  uint16_t raw_uvb = (data[2] << 8) | data[3];
  uint16_t raw_uvc = (data[4] << 8) | data[5];

  // Example scaling (datasheet-dependent)
  uva = raw_uva * 0.001f;
  uvb = raw_uvb * 0.001f;
  uvc = raw_uvc * 0.001f;

  return true;
}

void AS7331Component::update() {
  float uva, uvb, uvc;

  if (!read_channels_(uva, uvb, uvc))
    return;

  if (uva_) uva_->publish_state(uva);
  if (uvb_) uvb_->publish_state(uvb);
  if (uvc_) uvc_->publish_state(uvc);

  // UV Index calculation (CIE weighting)
  float uvi = (uva * 0.0029f) + (uvb * 0.058f) + (uvc * 0.002f);

  if (uvi_) uvi_->publish_state(uvi);
}

}  // namespace as7331
}  // namespace esphome
