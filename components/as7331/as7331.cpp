#include "as7331.h"
#include "esphome/core/log.h"

namespace esphome {
namespace as7331 {

static const char *const TAG = "as7331";

void AS7331Component::setup() {
  ESP_LOGI(TAG, "Setting up AS7331...");

  uint8_t agen = 0;
  if (!this->read_byte(REG_CFG_AGEN, &agen)) {
    ESP_LOGE(TAG, "I2C read failed (AGEN). Check wiring/address.");
    this->mark_failed();
    return;
  }

  if (!enter_cfg_mode_()) {
    ESP_LOGE(TAG, "Failed to enter CFG mode.");
    this->mark_failed();
    return;
  }

  if (!write_cfg_regs_()) {
    ESP_LOGE(TAG, "Failed to write configuration registers.");
    this->mark_failed();
    return;
  }

  if (!enter_meas_mode_()) {
    ESP_LOGE(TAG, "Failed to enter MEAS mode.");
    this->mark_failed();
    return;
  }

  // In CONT mode, start once to get the engine running.
  if (meas_mode_ == 0 /* cont */) {
    if (!start_measurement_()) {
      ESP_LOGW(TAG, "Failed to start CONT measurement (initial).");
    }
  }

  ESP_LOGI(TAG, "AS7331 setup complete.");
}

void AS7331Component::update() {
  if (this->is_failed()) return;

  // CMD mode: trigger measurement every update
  if (meas_mode_ == 1 /* cmd */) {
    if (!start_measurement_()) {
      ESP_LOGW(TAG, "Failed to start CMD measurement.");
      return;
    }
  }

  if (!wait_new_data_(2000)) {
    ESP_LOGW(TAG, "Timeout waiting for new data.");
    return;
  }

  // Read 16-bit results
  uint16_t m1 = 0, m2 = 0, m3 = 0;
  if (!read_u16_(REG_MEAS_MRES1, m1) || !read_u16_(REG_MEAS_MRES2, m2) || !read_u16_(REG_MEAS_MRES3, m3)) {
    ESP_LOGW(TAG, "Failed to read MRES registers.");
    return;
  }

  // Publish raw
  if (uva_ != nullptr) uva_->publish_state(static_cast<float>(m1));
  if (uvb_ != nullptr) uvb_->publish_state(static_cast<float>(m2));
  if (uvc_ != nullptr) uvc_->publish_state(static_cast<float>(m3));

  // Publish scaled irradiance if configured
  if (uva_irr_ != nullptr) uva_irr_->publish_state(static_cast<float>(m1) * uva_mult_);
  if (uvb_irr_ != nullptr) uvb_irr_->publish_state(static_cast<float>(m2) * uvb_mult_);
  if (uvc_irr_ != nullptr) uvc_irr_->publish_state(static_cast<float>(m3) * uvc_mult_);
}

bool AS7331Component::enter_cfg_mode_() {
  uint8_t osr = 0;
  if (!this->read_byte(REG_CFG_OSR, &osr)) return false;
  osr = (osr & ~OSR_DOS_MASK) | (DOS_CFG & OSR_DOS_MASK);
  return write_u8_(REG_CFG_OSR, osr);
}

bool AS7331Component::enter_meas_mode_() {
  uint8_t osr = 0;
  if (!this->read_byte(REG_CFG_OSR, &osr)) return false;
  osr = (osr & ~OSR_DOS_MASK) | (DOS_MEAS & OSR_DOS_MASK);
  return write_u8_(REG_CFG_OSR, osr);
}

bool AS7331Component::write_cfg_regs_() {
  // CREG1: gain in high nibble, time in low nibble
  const uint8_t creg1 = static_cast<uint8_t>((gain_ << 4) | (conv_time_ & 0x0F));
  if (!write_u8_(REG_CFG_CREG1, creg1)) return false;

  // CREG2: divider in bits 0..2, enable in bit 3
  uint8_t creg2 = 0;
  creg2 |= (divider_ & 0x07);
  if (en_div_) creg2 |= 0x08;
  if (!write_u8_(REG_CFG_CREG2, creg2)) return false;

  // CREG3: cclk in bits 0..1, measurement mode in bits 6..7
  uint8_t creg3 = 0;
  creg3 |= (cclk_ & 0x03);
  creg3 |= (meas_mode_ & 0x03) << 6;
  if (!write_u8_(REG_CFG_CREG3, creg3)) return false;

  return true;
}

bool AS7331Component::start_measurement_() {
  // In MEAS mode, set Start bit (SS). Some variants accept writing to OSR/OSRSTAT low byte.
  // We do: read OSR (0x00) and set bit7.
  uint8_t osr = 0;
  if (!this->read_byte(REG_CFG_OSR, &osr)) return false;
  osr |= OSR_SS;
  return write_u8_(REG_CFG_OSR, osr);
}

bool AS7331Component::wait_new_data_(uint32_t timeout_ms) {
  const uint32_t start = millis();
  while ((millis() - start) < timeout_ms) {
    uint16_t osrstat = 0;
    if (!read_u16_(REG_MEAS_OSRSTAT, osrstat)) return false;

    // New-data flag is commonly exposed in OSRSTAT upper bits; we use bit 11 as in typical AS733x behavior.
    // If your board uses a different flag bit, we can adjust after one debug log.
    if (osrstat & (1u << 11)) return true;

    delay(10);
  }
  return false;
}

bool AS7331Component::read_u16_(uint8_t reg, uint16_t &out) {
  uint8_t data[2] = {0, 0};
  if (!this->read_bytes(reg, data, 2)) return false;

  // If values look swapped, swap these two bytes.
  out = (static_cast<uint16_t>(data[1]) << 8) | data[0];
  return true;
}

bool AS7331Component::write_u8_(uint8_t reg, uint8_t val) {
  return this->write_byte(reg, val);
}

}  // namespace as7331
}  // namespace esphome
