#include "as7331.h"
#include "esphome/core/log.h"

namespace esphome {
namespace as7331 {

static const char *const TAG = "as7331";

/* === AS7331 Register Map (Datasheet) === */
static const uint8_t REG_OSR      = 0x00;
static const uint8_t REG_CREG1    = 0x01;
static const uint8_t REG_DATA     = 0x10;

/* === Helpers === */

// Gain: 1…2048 → register value 11…0
static uint8_t gain_to_reg(uint16_t gain) {
  uint8_t reg = 11;
  while (gain > 1 && reg > 0) {
    gain >>= 1;
    reg--;
  }
  return reg;
}

// Integration time: ms → log2(ms)
static uint8_t time_to_reg(uint16_t ms) {
  uint8_t reg = 0;
  while (ms > 1 && reg < 15) {
    ms >>= 1;
    reg++;
  }
  return reg;
}

/* === Device configuration === */

bool AS7331Component::configure_() {
  ESP_LOGI(TAG, "Configuring AS7331 (gain=%u, int=%ums)", gain_, integration_time_ms_);

  uint8_t gain_reg = gain_to_reg(gain_);
  uint8_t time_reg = time_to_reg(integration_time_ms_);

  // CREG1: GAIN[7:4], TIME[3:0]
  uint8_t creg1 = (gain_reg << 4) | (time_reg & 0x0F);

  if (!write_byte(REG_CREG1, creg1)) {
    ESP_LOGE(TAG, "Failed to write CREG1");
    return false;
  }

  return true;
}

/* === Measurement control === */

bool AS7331Component::start_measurement_() {
  // OSR: PD=0, DOS=MEAS, SS=1, CONT
  if (!write_byte(REG_OSR, 0x83)) {
    ESP_LOGE(TAG, "Failed to start measurement");
    return false;
  }

  measuring_ = true;
  ESP_LOGI(TAG, "Measurement started (continuous)");
  return true;
}

bool AS7331Component::stop_measurement_() {
  // SS=0 → stop, remain powered
  if (!write_byte(REG_OSR, 0x03)) {
    ESP_LOGE(TAG, "Failed to stop measurement");
    return false;
  }

  measuring_ = false;
  ESP_LOGI(TAG, "Measurement stopped");
  return true;
}

void AS7331Component::set_measurement_enabled(bool enable) {
  if (enable && !measuring_) {
    start_measurement_();
  } else if (!enable && measuring_) {
    stop_measurement_();
  }
}

/* === ESPHome hooks === */

void AS7331Component::setup() {
  ESP_LOGI(TAG, "Setting up AS7331");
  if (configure_()) {
    start_measurement_();
  }
}

bool AS7331Component::read_raw_(uint16_t &u, uint16_t &b, uint16_t &c) {
  uint8_t buf[6];

  if (!read_bytes(REG_DATA, buf, 6)) {
    ESP_LOGE(TAG, "Read failed");
    return false;
  }

  u = (buf[0] << 8) | buf[1];
  b = (buf[2] << 8) | buf[3];
  c = (buf[4] << 8) | buf[5];

  return true;
}

void AS7331Component::update() {
  if (!measuring_) {
    return;
  }

  uint16_t ru, rb, rc;
  if (!read_raw_(ru, rb, rc)) {
    return;
  }

  // Placeholder scaling (replace with datasheet calibration)
  float uva = ru * 0.001f;
  float uvb = rb * 0.001f;
  float uvc = rc * 0.001f;

  if (uva_) uva_->publish_state(uva);
  if (uvb_) uvb_->publish_state(uvb);
  if (uvc_) uvc_->publish_state(uvc);

  if (uvi_) {
    float uvi = (uva * 0.0029f) + (uvb * 0.058f);
    uvi_->publish_state(uvi);
  }
}

}  // namespace as7331
}  // namespace esphome
