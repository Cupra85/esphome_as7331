#include "as7331.h"
#include "esphome/core/log.h"

namespace esphome {
namespace as7331 {

static const char *TAG = "as7331";

/* === Register Map (SparkFun / Datasheet korrekt) === */
static const uint8_t REG_OSR    = 0x00;
static const uint8_t REG_AGEN   = 0x02;
static const uint8_t REG_CREG1  = 0x06;
static const uint8_t REG_CREG2  = 0x07;
static const uint8_t REG_CREG3  = 0x08;

/* 🔑 RESULT POINTER (NICHT einzelne Register!) */
static const uint8_t REG_RESULT = 0x10;

/* === OSR bits === */
static const uint8_t OSR_DOS_MEAS = 0x40;
static const uint8_t OSR_SS       = 0x20;

/* === Configuration === */
void AS7331Component::configure_() {
  ESP_LOGI(TAG, "Configuring AS7331");

  write_byte(REG_AGEN, 0x01);

  uint8_t creg1 = ((gain_ & 0x0F) << 4) | (integration_time_ & 0x0F);
  write_byte(REG_CREG1, creg1);

  write_byte(REG_CREG2, 0x00);
  write_byte(REG_CREG3, 0x00); // CONT
}

/* === Setup === */
void AS7331Component::setup() {
  configure_();

  // Start continuous measurement ONCE
  write_byte(REG_OSR, OSR_DOS_MEAS | OSR_SS);
}

/* === UPDATE === */
void AS7331Component::update() {
  uint8_t buf[6];

  // 🔑 ONE burst read – advances internal pointer
  if (!read_bytes(REG_RESULT, buf, 6)) {
    ESP_LOGW(TAG, "Failed to read result frame");
    return;
  }

  uint16_t uva = (uint16_t(buf[0]) << 8) | buf[1];
  uint16_t uvb = (uint16_t(buf[2]) << 8) | buf[3];
  uint16_t uvc = (uint16_t(buf[4]) << 8) | buf[5];

  if (uva_) uva_->publish_state(uva);
  if (uvb_) uvb_->publish_state(uvb);
  if (uvc_) uvc_->publish_state(uvc);
}

}  // namespace as7331
}  // namespace esphome
