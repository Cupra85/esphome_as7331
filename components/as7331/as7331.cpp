#include "as7331.h"
#include <cmath>

namespace esphome {
namespace as7331 {

// Kalibrierfaktoren (µW/cm² pro Count)
static constexpr float K_UVA = 0.030f;
static constexpr float K_UVB = 0.004f;
static constexpr float K_UVC = 0.001f;

void AS7331Component::update() {
  uint16_t raw_uva = this->read_uva();
  uint16_t raw_uvb = this->read_uvb();
  uint16_t raw_uvc = this->read_uvc();

  if (uva_raw_) uva_raw_->publish_state(raw_uva);
  if (uvb_raw_) uvb_raw_->publish_state(raw_uvb);
  if (uvc_raw_) uvc_raw_->publish_state(raw_uvc);

  float uva = raw_uva * K_UVA * 0.01f;
  float uvb = raw_uvb * K_UVB * 0.01f;
  float uvc = raw_uvc * K_UVC * 0.01f;

  if (uva < 0 || std::isnan(uva)) uva = 0;
  if (uvb < 0 || std::isnan(uvb)) uvb = 0;
  if (uvc < 0 || std::isnan(uvc)) uvc = 0;

  if (uva_) uva_->publish_state(uva);
  if (uvb_) uvb_->publish_state(uvb);
  if (uvc_) uvc_->publish_state(uvc);

  if (uv_index_) {
    float uv_index = (uva * 0.025f + uvb) / 0.025f;
    if (uv_index < 0 || std::isnan(uv_index)) uv_index = 0;
    uv_index_->publish_state(uv_index);
  }
}

// Platzhalter – hier bleibt deine bestehende I²C-Logik
uint16_t AS7331Component::read_uva() { return 0; }
uint16_t AS7331Component::read_uvb() { return 0; }
uint16_t AS7331Component::read_uvc() { return 0; }

}  // namespace as7331
}  // namespace esphome
