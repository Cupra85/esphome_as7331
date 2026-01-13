#include "as7331.h"
#include "esphome/core/log.h"
#include <algorithm>

namespace esphome {
namespace as7331 {

static const char *TAG = "as7331";

/* ================= Register ================= */
static const uint8_t REG_OSR   = 0x00;
static const uint8_t REG_CREG1 = 0x06;
static const uint8_t REG_CREG2 = 0x07;
static const uint8_t REG_CREG3 = 0x08;
static const uint8_t REG_BREAK = 0x09;
static const uint8_t REG_MRES1 = 0x02;

static const uint8_t OSR_SS     = 0x80;
static const uint8_t DOS_CONFIG = 0x02;
static const uint8_t DOS_MEAS   = 0x03;

/* =========================================================
 * Basis-LSB (nW/cm² per count)
 * gain = 0, int_time = 0, CCLK = 1.024 MHz
 * Quelle: AMS AS7331 Datasheet / SparkFun
 * ========================================================= */
static constexpr float LSB_BASE_UVA = 0.046f;
static constexpr float LSB_BASE_UVB = 0.052f;
static constexpr float LSB_BASE_UVC = 0.060f;

/* =========================================================
 * SparkFun Responsivity Factors
 * (entscheidend für realistische W/m²!)
 * ========================================================= */
static constexpr float RESP_UVA = 0.93f;
static constexpr float RESP_UVB = 1.00f;
static constexpr float RESP_UVC = 1.27f;

/* =========================================================
 * Skalierungsfunktionen
 * ========================================================= */
static inline float gain_factor(uint8_t gain) {
  return static_cast<float>(1 << gain);     // 2^gain
}

static inline float int_time_factor(uint8_t int_time) {
  return 1.0f / static_cast<float>(1 << int_time); // 1 / 2^int_time
}

/* ================= Profile ================= */
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
  ESP_LOGI(TAG, "Profile applied → gain=%u int_time=%u", gain_, int_time_);
}

/* ================= Auto-Gain / Auto-Time ================= */
void AS7331Component::auto_adjust_(uint16_t uva, uint16_t uvb, uint16_t uvc) {
  const uint16_t RAW_MAX = 50000;
  const uint16_t RAW_MIN = 1500;

  uint16_t peak = std::max({uva, uvb, uvc});
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

/* ================= Setup ================= */
void AS7331Component::setup() {
  write_byte(REG_OSR, DOS_CONFIG);
  delay(2);

  apply_profile_();

  write_byte(REG_CREG2, 0x40);  // CCLK = 1.024 MHz
  write_byte(REG_CREG3, 0x00);
  write_byte(REG_BREAK, 0x19);

  write_byte(REG_OSR, DOS_MEAS);
  delay(2);
  write_byte(REG_OSR, OSR_SS | DOS_MEAS);

  ESP_LOGI(TAG, "AS7331 started (SparkFun compatible, CONT mode)");
}

/* ================= Update ================= */
void AS7331Component::update() {
  uint8_t buf[6];
  if (!read_bytes(REG_MRES1, buf, 6)) return;

  uint16_t uva = (buf[1] << 8) | buf[0];
  uint16_t uvb = (buf[3] << 8) | buf[2];
  uint16_t uvc = (buf[5] << 8) | buf[4];

  if (uva == 0xFFFF || uvb == 0xFFFF || uvc == 0xFFFF) {
    ESP_LOGW(TAG, "Saturation detected");
    auto_adjust_(uva, uvb, uvc);
    return;
  }

  /* ===== RAW ===== */
  if (uva_raw_) uva_raw_->publish_state(uva);
  if (uvb_raw_) uvb_raw_->publish_state(uvb);
  if (uvc_raw_) uvc_raw_->publish_state(uvc);

  /* ===== Auto-Gain ===== */
  auto_adjust_(uva, uvb, uvc);

  /* ===== SparkFun-konforme Skalierung ===== */
  float scale =
      gain_factor(gain_) *
      int_time_factor(int_time_) *
      1e-5f;  // nW/cm² → W/m²

  float uva_wm2 = uva * LSB_BASE_UVA * scale * RESP_UVA;
  float uvb_wm2 = uvb * LSB_BASE_UVB * scale * RESP_UVB;
  float uvc_wm2 = uvc * LSB_BASE_UVC * scale * RESP_UVC;

  if (uva_wm2_) uva_wm2_->publish_state(uva_wm2);
  if (uvb_wm2_) uvb_wm2_->publish_state(uvb_wm2);
  if (uvc_wm2_) uvc_wm2_->publish_state(uvc_wm2);

  /* ===== UV Index (WHO / SparkFun-ähnlich) ===== */
  float uv_index =
      (uva_wm2 * 0.0025f) +
      (uvb_wm2 * 0.0100f);

  if (uv_index_) uv_index_->publish_state(uv_index);

  ESP_LOGD(TAG,
           "RAW U:%u B:%u C:%u | W/m² U:%.6f B:%.6f C:%.6f | UVI %.2f",
           uva, uvb, uvc,
           uva_wm2, uvb_wm2, uvc_wm2,
           uv_index);
}

}  // namespace as7331
}  // namespace esphome
