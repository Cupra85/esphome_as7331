#include "as7331.h"
#include "esphome/core/log.h"

namespace esphome {
namespace as7331 {

static const char *TAG = "as7331";

void AS7331Component::setup() {
  // Schreibe Konfiguration (Gain, Integration, Divider, Mode)
  write_cfg_();
}

void AS7331Component::update() {
  // Messung starten (CONT oder CMD – harmlos auch in CONT)
  start_measurement_();

  // Kurze Wartezeit, damit Register sicher aktualisiert sind
  delay(5);

  uint16_t m1 = 0;
  uint16_t m2 = 0;
  uint16_t m3 = 0;

  // Register IMMER lesen (keine Blockade mehr)
  if (!read_u16_(REG_MRES1, m1) ||
      !read_u16_(REG_MRES2, m2) ||
      !read_u16_(REG_MRES3, m3)) {
    ESP_LOGW(TAG, "Failed to read AS7331 measurement registers");
    return;
  }

  // Cache aktualisieren
  last_uva_ = m1;
  last_uvb_ = m2;
  last_uvc_ = m3;

  // Raw Counts publishen
  if (uva_) uva_->publish_state(m1);
  if (uvb_) uvb_->publish_state(m2);
  if (uvc_) uvc_->publish_state(m3);

  // Skalierte Werte (µW/cm²) publishen
  if (uva_irr_) uva_irr_->publish_state(m1 * uva_mult_);
  if (uvb_irr_) uvb_irr_->publish_state(m2 * uvb_mult_);
  if (uvc_irr_) uvc_irr_->publish_state(m3 * uvc_mult_);
}

bool AS7331Component::write_cfg_() {
  // CREG1: Gain (High-Nibble) | Integration Time (Low-Nibble)
  uint8_t creg1 = (gain_ << 4) | (conv_time_ & 0x0F);

  // CREG2: Divider + Enable
  uint8_t creg2 = (divider_ & 0x07);
  if (en_div_) creg2 |= 0x08;

  // CREG3: CCLK + Measurement Mode
  uint8_t creg3 = (cclk_ & 0x03) | ((meas_mode_ & 0x03) << 6);

  if (!write_byte(REG_CREG1, creg1)) return false;
  if (!write_byte(REG_CREG2, creg2)) return false;
  if (!write_byte(REG_CREG3, creg3)) return false;

  return true;
}

bool AS7331Component::start_measurement_() {
  // DOS = MEAS, SS = Start
  return write_byte(REG_OSR, DOS_MEAS | OSR_SS);
}

bool AS7331Component::read_u16_(uint8_t reg, uint16_t &out) {
  uint8_t data[2] = {0, 0};
  if (!read_bytes(reg, data, 2)) {
    return false;
  }

  // Little endian
  out = (static_cast<uint16_t>(data[1]) << 8) | data[0];
  return true;
}

}  // namespace as7331
}  // namespace esphome
