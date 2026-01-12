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

/* OSR */
static const uint8_t OSR_SS = 0x80;
static const uint8_t DOS_CONFIG = 0x02;
static const uint8_t DOS_MEAS   = 0x03;

/* GAIN index → real factor (Datasheet) */
static const uint16_t GAIN_FACTORS[12] = {
  2048, 1024, 512, 256, 128, 64, 32, 16, 8, 4, 2, 1
};

/* LSB values in nW/cm² per count for TIME=0..7 (UVA/UVB/UVC)
   Extracted from datasheet figures */
static const float UVA_LSB[8] = { 0.093, 0.047, 0.023, 0.012, 0.006, 0.003, 0.0015, 0.00075 };
static const float UVB_LSB[8] = { 0.110, 0.055, 0.027, 0.014, 0.007, 0.0035, 0.0018, 0.0009 };
static const float UVC_LSB[8] = { 0.130, 0.065, 0.033, 0.016, 0.008, 0.0040, 0.0020, 0.0010 };

void AS7331Component::setup() {
  ESP_LOGI(TAG, "AS7331 init");

  write_byte(REG_OSR, DOS_CONFIG);
  delay(2);

  write_byte(REG_CREG1, (gain_ << 4) | int_time_);
  write_byte(REG_CREG2, 0x40);
  write_byte(REG_CREG3, 0x00);
  write_byte(REG_BREAK, 0x19);

  write_byte(REG_OSR, DOS_MEAS);
  delay(2);
  write_byte(REG_OSR, OSR_SS | DOS_MEAS);

  ESP_LOGI(TAG, "AS7331 running (CONT)");
}

void AS7331Component::update() {
  uint8_t buf[6];
  if (!read_bytes(REG_MRES1, buf, 6)) return;

  uint16_t uva = (buf[1] << 8) | buf[0];
  uint16_t uvb = (buf[3] << 8) | buf[2];
  uint16_t uvc = (buf[5] << 8) | buf[4];

  if (uva_) uva_->publish_state(uva);
  if (uvb_) uvb_->publish_state(uvb);
  if (uvc_) uvc_->publish_state(uvc);

  float gain_factor = GAIN_FACTORS[gain_];

  /* nW/cm² → W/m² = ×1e-5 */
  float uva_wm2 = uva * UVA_LSB[int_time_] * 1e-5f / gain_factor;
  float uvb_wm2 = uvb * UVB_LSB[int_time_] * 1e-5f / gain_factor;
  float uvc_wm2 = uvc * UVC_LSB[int_time_] * 1e-5f / gain_factor;

  if (uva_wm2_) uva_wm2_->publish_state(uva_wm2);
  if (uvb_wm2_) uvb_wm2_->publish_state(uvb_wm2);
  if (uvc_wm2_) uvc_wm2_->publish_state(uvc_wm2);

  ESP_LOGD(TAG,
           "RAW %u %u %u | W/m² %.6f %.6f %.6f",
           uva, uvb, uvc,
           uva_wm2, uvb_wm2, uvc_wm2);
}

}  // namespace as7331
}  // namespace esphome
