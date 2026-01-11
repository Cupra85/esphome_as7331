#include "as7331.h"
#include "esphome/core/log.h"

namespace esphome {
namespace as7331 {

static const char *TAG = "as7331";

/* === AS7331 Register Map (DS001047) === */
static const uint8_t REG_OSR   = 0x00;
static const uint8_t REG_CREG1 = 0x01;
static const uint8_t REG_DATA  = 0x10;

/* === Helpers === */

// Gain: 1…2048 → encoded 11…0
static uint8_t gain_to_reg(uint16_t gain) {
  uint8_t reg = 11;
  while (gain > 1 && reg > 0) {
    gain >>= 1;
    reg--;
  }
  return reg;
}

// Integration time ms → log2(ms)
static uint8_t time_to_reg(uint16_t ms) {
  uint8_t reg = 0;
  while (ms > 1 && reg < 15) {
    ms >>= 1;
    reg++;
  }
  return reg;
}

/* === Configuration === */

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

/* === Measurement control (DATASHEET-KORREKT) === */

bool AS7331Component::start_measurement_() {
  // Trigger measurement: PD=0, DOS=1, SS=1
  if (!write_byte(REG_OSR, 0x60)) {
    ESP_LOGE(TAG, "Failed to trigger measurement");
    return false;
  }

  delay(2);  // SS is edge-triggered

  // Run mode: PD=0, DOS=1, SS=0
  if (!write_byte(REG_OSR, 0x40)) {
    ESP_LOGE(TAG, "Failed to enter run mode");
    return false;
  }

  measuring_ = true;
  ESP_LOGI(TAG, "AS7331 measurement running");
  return true;
}

bool AS7331Component::stop_measurement_() {
  // Power down: PD=1
  if (!write_byte(REG_OSR, 0x80)) {
    ESP_LOGE(TAG, "Failed to power down AS7331");
    return false;
  }

  measuring_ = false;
  ESP_LOGI(TAG, "AS7331 powered down");
  return true;
}

void AS7331Component::set_measurement_enabled(bool enable) {
  if (enable && !measuring_) {
    start_measurement_();
  } else if (!enable && measuring_) {
    stop_measurement_();
  }
}

/* === ESPHome lifecycle === */

void AS7331Component::setup() {
  ESP_LOGI(TAG, "Setting up AS7331");

  if (!configure_()) {
    ESP_LOGE(TAG, "AS7331 configuration failed");
    return;
  }

  // AUTO-START after boot
  start_measurement_();
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
  if (!measuring_) return;

  uint16_t ru, rb, rc;
  if (!read_raw_(ru, rb, rc)) return;

  // Placeholder scaling (can be calibrated later)
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
