#include "as7331.h"
#include "esphome/core/log.h"

namespace esphome {
namespace as7331 {

static const char *TAG = "as7331";

/* Register */
static constexpr uint8_t REG_OSR   = 0x00;
static constexpr uint8_t REG_MRES1 = 0x02;
static constexpr uint8_t REG_CREG1 = 0x06;
static constexpr uint8_t REG_CREG3 = 0x08;
static constexpr uint8_t REG_BREAK = 0x09;

/* OSR */
static constexpr uint8_t OSR_CONFIG = 0x02;
static constexpr uint8_t OSR_MEAS   = 0x03;
static constexpr uint8_t OSR_START  = 0x83;

/* Typische Responsivität laut Datasheet (counts / µW/cm²) */
static constexpr float RESP_UVA = 0.188f;
static constexpr float RESP_UVB = 0.170f;
static constexpr float RESP_UVC = 0.388f;

void AS7331Component::setup() {
  ESP_LOGI(TAG, "Setting up AS7331");

  // CONFIG Mode
  this->write_byte(REG_OSR, OSR_CONFIG);
  delay(2);

  // Gain + Integration Time
  this->write_byte(REG_CREG1, (gain_ << 4) | (int_time_ & 0x0F));

  // Clock: 1.024 MHz
  this->write_byte(REG_CREG3, 0x00);

  // Pause time
  this->write_byte(REG_BREAK, 0x19);

  // Measurement Mode
  this->write_byte(REG_OSR, OSR_MEAS);
  delay(2);

  // Start CONT measurement
  this->write_byte(REG_OSR, OSR_START);

  ESP_LOGI(TAG, "AS7331 running in CONT mode");
}

void AS7331Component::update() {
  uint8_t buf[6];
  if (!this->read_bytes(REG_MRES1, buf, 6)) {
    ESP_LOGW(TAG, "Failed to read measurement registers");
    return;
  }

  uint16_t uva = (buf[1] << 8) | buf[0];
  uint16_t uvb = (buf[3] << 8) | buf[2];
  uint16_t uvc = (buf[5] << 8) | buf[4];

  if (uva_raw_) uva_raw_->publish_state(uva);
  if (uvb_raw_) uvb_raw_->publish_state(uvb);
  if (uvc_raw_) uvc_raw_->publish_state(uvc);

  // Integrationszeit in Sekunden
  const float tconv = (1 << int_time_) / 1000.0f;

  // µW/cm² → W/m²
  float uva_w = (uva / RESP_UVA) / tconv * 0.01f;
  float uvb_w = (uvb / RESP_UVB) / tconv * 0.01f;
  float uvc_w = (uvc / RESP_UVC) / tconv * 0.01f;

  if (uva_) uva_->publish_state(uva_w);
  if (uvb_) uvb_->publish_state(uvb_w);
  if (uvc_) uvc_->publish_state(uvc_w);

  // WHO UV Index (vereinfachte Formel)
  float uv_index = (uva_w * 0.0025f) + (uvb_w * 0.010f);
  if (uv_index_) uv_index_->publish_state(uv_index);
}

void AS7331Component::dump_config() {
  ESP_LOGCONFIG(TAG, "AS7331 UV Sensor");
  LOG_I2C_DEVICE(this);
  ESP_LOGCONFIG(TAG, "  Gain: %u", gain_);
  ESP_LOGCONFIG(TAG, "  Integration time: %u", int_time_);
}

}  // namespace as7331
}  // namespace esphome
