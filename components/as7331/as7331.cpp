#include "as7331.h"
#include "esphome/core/log.h"

namespace esphome {
namespace as7331 {

static const char *TAG = "as7331";

/* ===== Register Map (SparkFun) ===== */
static const uint8_t REG_OSR    = 0x00;
static const uint8_t REG_AGEN   = 0x02;
static const uint8_t REG_CREG1  = 0x06;
static const uint8_t REG_CREG2  = 0x07;
static const uint8_t REG_CREG3  = 0x08;

static const uint8_t REG_MRES1  = 0x10;
static const uint8_t REG_MRES2  = 0x12;
static const uint8_t REG_MRES3  = 0x14;

/* ===== OSR bits ===== */
static const uint8_t OSR_DOS_MEAS = 0x40;
static const uint8_t OSR_SS       = 0x20;

/* ===== Configuration ===== */
void AS7331Component::configure_() {
  ESP_LOGI(TAG, "Configuring AS7331");

  // Enable analog frontend
  write_byte(REG_AGEN, 0x01);

  // Gain + Integration Time
  uint8_t creg1 = ((gain_ & 0x0F) << 4) | (integration_time_ & 0x0F);
  write_byte(REG_CREG1, creg1);

  write_byte(REG_CREG2, 0x00); // Divider
  write_byte(REG_CREG3, 0x00); // CONT mode
}

/* ===== Start ONE measurement ===== */
void AS7331Component::start_measurement_() {
  write_byte(REG_OSR, OSR_DOS_MEAS | OSR_SS);
}

/* ===== Setup ===== */
void AS7331Component::setup() {
  ESP_LOGI(TAG, "AS7331 setup");

  configure_();
}

/* ===== Update = EXACT SparkFun loop ===== */
void AS7331Component::update() {
  // 1) START measurement (EVERY cycle!)
  start_measurement_();

  // 2) Wait for integration to finish
  uint16_t integration_ms = 1 << integration_time_;
  delay(integration_ms + 2);

  // 3) Read results
  uint8_t buf[2];
  uint16_t uva, uvb, uvc;

  if (!read_bytes(REG_MRES1, buf, 2)) return;
  uva = (uint16_t(buf[0]) << 8) | buf[1];

  if (!read_bytes(REG_MRES2, buf, 2)) return;
  uvb = (uint16_t(buf[0]) << 8) | buf[1];

  if (!read_bytes(REG_MRES3, buf, 2)) return;
  uvc = (uint16_t(buf[0]) << 8) | buf[1];

  // 4) Publish
  if (uva_) uva_->publish_state(uva);
  if (uvb_) uvb_->publish_state(uvb);
  if (uvc_) uvc_->publish_state(uvc);
}

}  // namespace as7331
}  // namespace esphome
