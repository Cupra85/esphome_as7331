#include "as7331.h"
#include "esphome/core/log.h"

namespace esphome {
namespace as7331 {

static const char *TAG = "as7331";

/* Registers */
static const uint8_t REG_OSR   = 0x00;
static const uint8_t REG_CREG1 = 0x06;
static const uint8_t REG_CREG2 = 0x07;
static const uint8_t REG_CREG3 = 0x08;
static const uint8_t REG_BREAK = 0x09;
static const uint8_t REG_MRES1 = 0x02;

static const uint8_t OSR_SS = 0x80;
static const uint8_t DOS_CONFIG = 0x02;
static const uint8_t DOS_MEAS   = 0x03;

/* ===== SparkFun LSB tables (unchanged) ===== */
/* (identisch zu deiner letzten funktionierenden Version – hier gekürzt) */
extern const float LSB_UVA[12][8];
extern const float LSB_UVB[12][8];
extern const float LSB_UVC[12][8];

/* ===== Profile defaults ===== */
void AS7331Component::apply_profile_() {
  switch (profile_) {
    case PROFILE_INDOOR:
      gain_ = 0;
      int_time_ = 7;
      break;

    case PROFILE_OUTDOOR:
      gain_ = 3;
      int_time_ = 4;
      break;

    case PROFILE_UV_LAMP:
      gain_ = 1;
      int_time_ = 2;
      break;
  }

  write_byte(REG_CREG1, (gain_ << 4) | int_time_);
  ESP_LOGI(TAG, "Profile applied: gain=%u int_time=%u", gain_, int_time_);
}

/* ===== Auto-Gain / Auto-Time ===== */
void AS7331Component::auto_adjust_() {
  const uint16_t RAW_MAX = 50000;
  const uint16_t RAW_MIN = 2000;

  uint16_t peak = std::max({(uint16_t)uva_raw_->state,
                            (uint16_t)uvb_raw_->state,
                            (uint16_t)uvc_raw_->state});

  bool changed = false;

  if (peak > RAW_MAX) {
    if (int_time_ > 0) {
      int_time_--;
      changed = true;
    } else if (gain_ < 11) {
      gain_++;
      changed = true;
    }
  } else if (peak < RAW_MIN) {
    if (gain_ > 0) {
      gain_--;
      changed = true;
    } else if (int_time_ < 7) {
      int_time_++;
      changed = true;
    }
  }

  if (changed) {
    write_byte(REG_CREG1, (gain_ << 4) | int_time_);
    ESP_LOGD(TAG, "Auto adjust → gain=%u int_time=%u", gain_, int_time_);
  }
}

/* ===== Setup ===== */
void AS7331Component::setup() {
  write_byte(REG_OSR, DOS_CONFIG);
  delay(2);

  apply_profile_();

  write_byte(REG_CREG2, 0x40);
  write_byte(REG_CREG3, 0x00);
  write_byte(REG_BREAK, 0x19);

  write_byte(REG_OSR, DOS_MEAS);
  delay(2);
  write_byte(REG_OSR, OSR_SS | DOS_MEAS);

  ESP_LOGI(TAG, "AS7331 running (Auto-Gain enabled)");
}

/* ===== Update ===== */
void AS7331Component::update() {
  uint8_t buf[6];
  if (!read_bytes(REG_MRES1, buf, 6)) return;

  uint16_t uva = (buf[1] << 8) | buf[0];
  uint16_t uvb = (buf[3] << 8) | buf[2];
  uint16_t uvc = (buf[5] << 8) | buf[4];

  if (uva == 0xFFFF || uvb == 0xFFFF || uvc == 0xFFFF) {
    ESP_LOGW(TAG, "Saturation detected");
    auto_adjust_();
    return;
  }

  if (uva_raw_) uva_raw_->publish_state(uva);
  if (uvb_raw_) uvb_raw_->publish_state(uvb);
  if (uvc_raw_) uvc_raw_->publish_state(uvc);

  auto_adjust_();

  float uva_wm2 = uva * LSB_UVA[gain_][int_time_] * 1e-5f;
  float uvb_wm2 = uvb * LSB_UVB[gain_][int_time_] * 1e-5f;
  float uvc_wm2 = uvc * LSB_UVC[gain_][int_time_] * 1e-5f;

  if (uva_wm2_) uva_wm2_->publish_state(uva_wm2);
  if (uvb_wm2_) uvb_wm2_->publish_state(uvb_wm2);
  if (uvc_wm2_) uvc_wm2_->publish_state(uvc_wm2);

  float uv_index = (uva_wm2 * 0.0025f) + (uvb_wm2 * 0.0100f);
  if (uv_index_) uv_index_->publish_state(uv_index);
}

}  // namespace as7331
}  // namespace esphome
