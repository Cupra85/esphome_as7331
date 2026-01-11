#include "as7331.h"
#include "esphome/core/log.h"

namespace esphome {
namespace as7331 {

static const char *TAG = "as7331";

/* === AS7331 REGISTER MAP (WIE IM FUNKTIONIERENDEN REPO) === */
static const uint8_t REG_OSR    = 0x00;
static const uint8_t REG_CREG1  = 0x06;
static const uint8_t REG_CREG2  = 0x07;
static const uint8_t REG_CREG3  = 0x08;

static const uint8_t REG_MRES1  = 0x10; // UVA
static const uint8_t REG_MRES2  = 0x12; // UVB
static const uint8_t REG_MRES3  = 0x14; // UVC

/* === OSR bits === */
static const uint8_t DOS_MEAS = 0x40;
static const uint8_t OSR_SS   = 0x20;

/* === CONFIGURATION === */

void AS7331Component::configure_() {
  ESP_LOGI(TAG, "Configuring AS7331");

  // CREG1: GAIN[7:4] | TIME[3:0]
  uint8_t creg1 = ((gain_ & 0x0F) << 4) | (integration_time_ & 0x0F);
  write_byte(REG_CREG1, creg1);

  // CREG2: Divider (default 0)
  write_byte(REG_CREG2, 0x00);

  // CREG3: Measurement mode CONT (0)
  write_byte(REG_CREG3, 0x00);
}

/* === MEASUREMENT CONTROL === */

void AS7331Component::start_measurement_() {
  // Trigger measurement (CONT mode already set)
  write_byte(REG_OSR, DOS_MEAS | OSR_SS);
  measuring_ = true;
  ESP_LOGI(TAG, "AS7331 measurement started (CONT)");
}

void AS7331Component::stop_measurement_() {
  // Stop measurement
  write_byte(REG_OSR, 0x00);
  measuring_ = false;
  ESP_LOGI(TAG, "AS7331 measurement stopped");
}

void AS7331Component::set_measurement_enabled(bool enable) {
  if (enable && !measuring_) {
    start_measurement_();
  } else if (!enable && measuring_) {
    stop_measurement_();
  }
}

/* === ESPHOME LIFECYCLE === */

void AS7331Component::setup() {
  ESP_LOGI(TAG, "Setting up AS7331");

  configure_();

  // AUTO START CONT MODE
  start_measurement_();
}

void AS7331Component::update() {
  if (!measuring_) return;

  uint16_t uva = read_byte(REG_MRES1) << 8 | read_byte(REG_MRES1 + 1);
  uint16_t uvb = read_byte(REG_MRES2) << 8 | read_byte(REG_MRES2 + 1);
  uint16_t uvc = read_byte(REG_MRES3) << 8 | read_byte(REG_MRES3 + 1);

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
