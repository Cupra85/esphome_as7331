#include "as7331.h"
#include "esphome/core/log.h"

namespace esphome {
namespace as7331 {

static const char *TAG = "as7331";

/* Registers */
static constexpr uint8_t REG_OSR   = 0x00;
static constexpr uint8_t REG_CREG1 = 0x06;
static constexpr uint8_t REG_CREG2 = 0x07;
static constexpr uint8_t REG_CREG3 = 0x08;
static constexpr uint8_t REG_BREAK = 0x09;

/* IMPORTANT: TEMP register block */
static constexpr uint8_t REG_TEMP  = 0x0A;

/* OSR */
static constexpr uint8_t OSR_CONFIG = 0x02;
static constexpr uint8_t OSR_MEAS   = 0x03;
static constexpr uint8_t OSR_START  = 0x83;

/* Status */
static constexpr uint8_t STATUS_NDATA = 0x08;

/* === FIXED measurement parameters (wie im funktionierenden Code) === */
static constexpr uint8_t GAIN     = 0;  // 0..11
static constexpr uint8_t INT_TIME = 7;  // 0..7

/* LSB tables – unverändert */
static const float LSB_UVA[12][8] = { /* deine Tabelle */ };
static const float LSB_UVB[12][8] = { /* deine Tabelle */ };
static const float LSB_UVC[12][8] = { /* deine Tabelle */ };

void AS7331Component::setup() {
  write_byte(REG_OSR, OSR_CONFIG);
  delay(3);

  write_byte(REG_CREG1, (GAIN << 4) | INT_TIME);
  write_byte(REG_CREG2, 0x40);
  write_byte(REG_CREG3, 0x00);
  write_byte(REG_BREAK, 0x19);

  write_byte(REG_OSR, OSR_MEAS);
  delay(3);
  write_byte(REG_OSR, OSR_START);

  ESP_LOGI(TAG, "AS7331 started (TEMP-sync, CONT mode)");
}

void AS7331Component::update() {
  uint8_t osr[2];
  if (!read_bytes(REG_OSR, osr, 2)) return;
  if (!(osr[1] & STATUS_NDATA)) return;

  /* === CRITICAL FIX === */
  uint8_t raw[8];
  if (!read_bytes(REG_TEMP, raw, 8)) return;

  uint16_t uva = (raw[3] << 8) | raw[2];
  uint16_t uvb = (raw[5] << 8) | raw[4];
  uint16_t uvc = (raw[7] << 8) | raw[6];

  if (uva_raw_) uva_raw_->publish_state(uva);
  if (uvb_raw_) uvb_raw_->publish_state(uvb);
  if (uvc_raw_) uvc_raw_->publish_state(uvc);

  float uva_wm2 = uva * LSB_UVA[GAIN][INT_TIME] * 0.01f;
  float uvb_wm2 = uvb * LSB_UVB[GAIN][INT_TIME] * 0.01f;
  float uvc_wm2 = uvc * LSB_UVC[GAIN][INT_TIME] * 0.01f;

  if (uva_wm2_) uva_wm2_->publish_state(uva_wm2);
  if (uvb_wm2_) uvb_wm2_->publish_state(uvb_wm2);
  if (uvc_wm2_) uvc_wm2_->publish_state(uvc_wm2);

  float uv_index =
      (uva_wm2 * 0.0025f) +
      (uvb_wm2 * 0.0100f);

  if (uv_index_) uv_index_->publish_state(uv_index);
}

}  // namespace as7331
}  // namespace esphome
