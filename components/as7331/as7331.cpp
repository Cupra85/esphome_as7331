#include "as7331.h"
#include "esphome/core/log.h"
#include <algorithm>

namespace esphome {
namespace as7331 {

static const char *TAG = "as7331";

static const uint8_t REG_MRES1 = 0x02;

static constexpr float LSB_BASE_UVA = 0.046f;
static constexpr float LSB_BASE_UVB = 0.052f;
static constexpr float LSB_BASE_UVC = 0.060f;

static inline float gain_factor(uint8_t gain) {
  return static_cast<float>(1 << gain);
}

static inline float int_time_factor(uint8_t int_time) {
  return 1.0f / static_cast<float>(1 << int_time);
}

void AS7331Component::setup() {
  ESP_LOGI(TAG, "AS7331 setup");
}

void AS7331Component::update() {
  uint8_t buf[6];
  if (!read_bytes(REG_MRES1, buf, 6)) return;

  uint16_t uva = (buf[1] << 8) | buf[0];
  uint16_t uvb = (buf[3] << 8) | buf[2];
  uint16_t uvc = (buf[5] << 8) | buf[4];

  float scale =
      gain_factor(gain_) *
      int_time_factor(int_time_) *
      1e-5f;

  float uva_wm2 = uva * LSB_BASE_UVA * scale;
  float uvb_wm2 = uvb * LSB_BASE_UVB * scale;
  float uvc_wm2 = uvc * LSB_BASE_UVC * scale;

  if (uva_raw_) uva_raw_->publish_state(uva);
  if (uvb_raw_) uvb_raw_->publish_state(uvb);
  if (uvc_raw_) uvc_raw_->publish_state(uvc);

  if (uva_wm2_) uva_wm2_->publish_state(uva_wm2);
  if (uvb_wm2_) uvb_wm2_->publish_state(uvb_wm2);
  if (uvc_wm2_) uvc_wm2_->publish_state(uvc_wm2);
}

}  // namespace as7331
}  // namespace esphome
