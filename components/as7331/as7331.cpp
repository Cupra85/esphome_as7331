#include "as7331.h"
#include "esphome/core/log.h"

namespace esphome {
namespace as7331 {

static const char *TAG = "as7331";

/* Registers (AS7331 datasheet) */
static const uint8_t REG_OSR      = 0x00;
static const uint8_t REG_CREG1    = 0x01;
static const uint8_t REG_DATA     = 0x10;

/* ---------- Helpers ---------- */

static uint8_t gain_to_reg(uint16_t gain) {
  // Valid gains: 1..2048 → encoded 11..0
  uint8_t g = 11;
  while (gain > 1 && g > 0) {
    gain >>= 1;
    g--;
  }
  return g;
}

static uint8_t time_to_reg(uint16_t ms) {
  uint8_t t = 0;
  while (ms > 1 && t < 15) {
    ms >>= 1;
    t++;
  }
  return t;
}

/* ---------- Device ---------- */

bool AS7331Component::configure_() {
  ESP_LOGI(TAG, "Configuring AS7331 (gain=%u, int=%ums)", gain_, integration_time_ms_);

  uint8_t gain_reg = gain_to_reg(gain_);
  uint8_t time_reg = time_to_reg(integration_time_ms_);

  // CREG1: GAIN[7:4], TIME[3:0]
  uint8_t creg1 = (gain_reg << 4) | (time_reg & 0x0F);

  if (!write_byte(REG_CREG1, creg1)) {
    ESP_LOGE(TAG, "CREG1 write failed");
    return false;
  }
  return true;
}

void AS7331Component::start_measurement() {
  // OSR: PD=0, DOS=MEAS, SS=1, CONT
  write_byte(REG_OSR, 0x83);
  ESP_LOGI(TAG, "Measurement started (continuous)");
}

void AS7331Component::stop_measurement() {
  write_byte(REG_OSR, 0x03);
  ESP_LOGI(TAG, "Measurement stopped");
}

void AS7331Component::setup() {
  configure_();
  start_measurement();
}

bool AS7331Component::read_raw_(uint16_t &u, uint16_t &b, uint16_t &c) {
  uint8_t buf[6];
  if (!read_bytes(REG_DATA, buf, 6))
    return false;

  u = (buf[0] << 8) | buf[1];
  b = (buf[2] << 8) | buf[3];
  c = (buf[4] << 8) | buf[5];
  return true;
}

void AS7331Component::update() {
  uint16_t ru, rb, rc;
  if (!read_raw_(ru, rb, rc))
    return;

  float uva = ru * 0.001f;
  float uvb = rb * 0.001f;
  float uvc = rc * 0.001f;

  if (uva_) uva_->publish_state(uva);
  if (uvb_) uvb_->publish_state(uvb);
  if (uvc_) uvc_->publish_state(uvc);

  if (uvi_)
    uvi_->publish_state((uva * 0.0029f) + (uvb * 0.058f));
}

}  // namespace as7331
}  // namespace esphome
