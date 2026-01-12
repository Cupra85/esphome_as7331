#include "as7331.h"
#include "esphome/core/log.h"

namespace esphome {
namespace as7331 {

static const char *TAG = "as7331";

/* =========================================================
 * REGISTER MAP (SparkFun / Datenblatt)
 * ========================================================= */
static const uint8_t REG_OSR    = 0x00;
static const uint8_t REG_AGEN   = 0x02;
static const uint8_t REG_CREG1  = 0x06;
static const uint8_t REG_CREG2  = 0x07;
static const uint8_t REG_CREG3  = 0x08;

static const uint8_t REG_MRES1  = 0x10;
static const uint8_t REG_MRES2  = 0x12;
static const uint8_t REG_MRES3  = 0x14;

/* =========================================================
 * OSR BITS
 * ========================================================= */
static const uint8_t OSR_DOS_MEAS = 0x40;
static const uint8_t OSR_SS       = 0x20;

/* =========================================================
 * CONFIGURATION (SparkFun-konform)
 * ========================================================= */
void AS7331Component::configure_() {
  ESP_LOGI(TAG, "Configuring AS7331");

  // Enable analog front end
  write_byte(REG_AGEN, 0x01);

  // Gain + Integration
  uint8_t creg1 = ((gain_ & 0x0F) << 4) | (integration_time_ & 0x0F);
  write_byte(REG_CREG1, creg1);

  // Divider default
  write_byte(REG_CREG2, 0x00);

  // Measurement mode = CONT
  write_byte(REG_CREG3, 0x00);
}

/* =========================================================
 * SETUP
 * ========================================================= */
void AS7331Component::setup() {
  ESP_LOGI(TAG, "Starting AS7331 continuous measurement");

  configure_();

  // 🔑 EINMAL starten (CONT)
  write_byte(REG_OSR, OSR_DOS_MEAS | OSR_SS);
}

/* =========================================================
 * UPDATE = ECHTE DAUERMESSUNG
 * ========================================================= */
void AS7331Component::update() {
  uint8_t status;

  // 🔑 WICHTIG: STATUS/OSR lesen → Data-Ready-Latch löschen
  if (!read_bytes(REG_OSR, &status, 1)) {
    ESP_LOGW(TAG, "Failed to read OSR");
    return;
  }

  uint8_t buf[2];
  uint16_t uva, uvb, uvc;

  if (!read_bytes(REG_MRES1, buf, 2)) return;
  uva = (uint16_t(buf[0]) << 8) | buf[1];

  if (!read_bytes(REG_MRES2, buf, 2)) return;
  uvb = (uint16_t(buf[0]) << 8) | buf[1];

  if (!read_bytes(REG_MRES3, buf, 2)) return;
  uvc = (uint16_t(buf[0]) << 8) | buf[1];

  if (uva_) uva_->publish_state(uva);
  if (uvb_) uvb_->publish_state(uvb);
  if (uvc_) uvc_->publish_state(uvc);
}

}  // namespace as7331
}  // namespace esphome
