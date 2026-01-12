#include "as7331.h"
#include "esphome/core/log.h"

namespace esphome {
namespace as7331 {

static const char *TAG = "as7331";

/* Register map (Measurement state) */
static const uint8_t REG_OSR   = 0x00;
static const uint8_t REG_CREG1 = 0x06;
static const uint8_t REG_CREG2 = 0x07;
static const uint8_t REG_CREG3 = 0x08;
static const uint8_t REG_BREAK = 0x09;

/* Result registers (measurement state) */
static const uint8_t REG_MRES1 = 0x02;  // UVA (LSB first)

/* OSR bits */
static const uint8_t OSR_SS = 0x80;
static const uint8_t DOS_CONFIG = 0x02;
static const uint8_t DOS_MEAS   = 0x03;

/* Calibration constants (SparkFun typical values) */
static constexpr float UVA_CAL = 0.0011f;
static constexpr float UVB_CAL = 0.00125f;
static constexpr float UVC_CAL = 0.0014f;

void AS7331Component::dump_config() {
  ESP_LOGCONFIG(TAG, "AS7331:");
  LOG_I2C_DEVICE(this);
  ESP_LOGCONFIG(TAG, "  Gain: %u (2^gain)", gain_);
  ESP_LOGCONFIG(TAG, "  Integration time: %u (2^n ms)", int_time_);
  LOG_UPDATE_INTERVAL(this);
}

void AS7331Component::setup() {
  ESP_LOGI(TAG, "Initializing AS7331");

  /* Enter CONFIG state */
  write_byte(REG_OSR, DOS_CONFIG);
  delay(2);

  /* Gain + integration time */
  uint8_t creg1 = (gain_ << 4) | int_time_;
  write_byte(REG_CREG1, creg1);

  /* Divider disabled, EN_TM default */
  write_byte(REG_CREG2, 0x40);

  /* CONT mode, CCLK = 1.024 MHz */
  write_byte(REG_CREG3, 0x00);

  /* Small break between conversions */
  write_byte(REG_BREAK, 0x19);

  /* Enter MEAS state */
  write_byte(REG_OSR, DOS_MEAS);
  delay(2);

  /* Start continuous measurement */
  write_byte(REG_OSR, OSR_SS | DOS_MEAS);

  ESP_LOGI(TAG, "AS7331 running in CONT mode");
}

void AS7331Component::update() {
  uint8_t buf[6];

  if (!read_bytes(REG_MRES1, buf, sizeof(buf))) {
    ESP_LOGW(TAG, "Failed to read result registers");
    return;
  }

  /* LSB first */
  uint16_t uva = (uint16_t(buf[1]) << 8) | buf[0];
  uint16_t uvb = (uint16_t(buf[3]) << 8) | buf[2];
  uint16_t uvc = (uint16_t(buf[5]) << 8) | buf[4];

  if (uva_) uva_->publish_state(uva);
  if (uvb_) uvb_->publish_state(uvb);
  if (uvc_) uvc_->publish_state(uvc);

  float gain_factor = 1 << gain_;
  float int_factor  = 1 << int_time_;

  float uva_wm2 = (uva / int_factor) / gain_factor * UVA_CAL;
  float uvb_wm2 = (uvb / int_factor) / gain_factor * UVB_CAL;
  float uvc_wm2 = (uvc / int_factor) / gain_factor * UVC_CAL;

  if (uva_wm2_) uva_wm2_->publish_state(uva_wm2);
  if (uvb_wm2_) uvb_wm2_->publish_state(uvb_wm2);
  if (uvc_wm2_) uvc_wm2_->publish_state(uvc_wm2);

  ESP_LOGD(TAG, "RAW: %u %u %u | W/m²: %.4f %.4f %.4f",
           uva, uvb, uvc, uva_wm2, uvb_wm2, uvc_wm2);
}

}  // namespace as7331
}  // namespace esphome
