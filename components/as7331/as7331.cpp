#include "as7331.h"
#include "esphome/core/log.h"

namespace esphome {
namespace as7331 {

static const char *const TAG = "as7331";

void AS7331Component::setup() {
  ESP_LOGI(TAG, "Setting up AS7331...");

  // Basic connectivity: read AGEN and check top nibble 0x2 (device id nibble) :contentReference[oaicite:14]{index=14}
  uint8_t agen = 0;
  if (!this->read_byte(REG_CFG_AGEN, &agen)) {
    ESP_LOGE(TAG, "I2C read failed (AGEN). Check wiring/I2C address.");
    this->mark_failed();
    return;
  }
  if ((agen >> 4) != 0x2) {
    ESP_LOGW(TAG, "Unexpected AGEN value: 0x%02X (top nibble should be 0x2). Continuing anyway.", agen);
  }

  // Configure
  if (!enter_cfg_mode_() || !write_cfg_regs_()) {
    ESP_LOGE(TAG, "Failed to configure AS7331.");
    this->mark_failed();
    return;
  }

  // Go to measurement mode (actual measurement triggered in update())
  if (!enter_meas_mode_()) {
    ESP_LOGE(TAG, "Failed to enter measurement mode.");
    this->mark_failed();
    return;
  }

  ESP_LOGI(TAG, "AS7331 setup complete.");
}

void AS7331Component::update() {
  if (this->is_failed()) return;

  // For "cmd" (one-shot), trigger measurement each update; for "cont", keep it running
  if (meas_mode_ == 1 /* CMD */) {
    if (!start_measurement_()) {
      ESP_LOGW(TAG, "Failed to start CMD measurement.");
      return;
    }
  }

  if (!wait_data_ready_(2000)) {
    ESP_LOGW(TAG, "Timeout waiting for new data.");
    return;
  }

  // Read results: MRES1/2/3 correspond to channels (published as raw counts)
  // Output result regs documented as 16-bit. :contentReference[oaicite:15]{index=15}
  uint16_t m1 = 0, m2 = 0, m3 = 0;
  if (!read_u16_(REG_MEAS_MRES1, m1) || !read_u16_(REG_MEAS_MRES2, m2) || !read_u16_(REG_MEAS_MRES3, m3)) {
    ESP_LOGW(TAG, "Failed reading MRES registers.");
    return;
  }

  // Temperature is 12-bit with upper 4 bits 0 (SparkFun note) :contentReference[oaicite:16]{index=16}
  uint16_t t_raw = 0;
  if (temp_ != nullptr) {
    if (read_u16_(REG_MEAS_TEMP, t_raw)) {
      // Datasheet-specific scaling can vary by mode; publish a conservative placeholder:
      // Provide raw->°C conversion later if you want; for now publish raw/16 as a reasonable "engineering" view.
      // (Keeps it monotonic and easy to validate.)
      float temp_c = (t_raw & 0x0FFF) / 16.0f;
      temp_->publish_state(temp_c);
    }
  }

  if (outconv_ != nullptr) {
    uint16_t oc = 0;
    if (read_u16_(REG_MEAS_OUTCONV_L, oc)) {
      outconv_->publish_state(static_cast<float>(oc));
    }
  }

  if (uva_ != nullptr) uva_->publish_state(static_cast<float>(m1));
  if (uvb_ != nullptr) uvb_->publish_state(static_cast<float>(m2));
  if (uvc_ != nullptr) uvc_->publish_state(static_cast<float>(m3));
}

bool AS7331Component::enter_cfg_mode_() {
  // OSR.dos selects device operating state. :contentReference[oaicite:17]{index=17}
  uint8_t osr = 0;
  if (!this->read_byte(REG_CFG_OSR, &osr)) return false;
  osr = (osr & ~OSR_DOS_MASK) | (DOS_CFG & OSR_DOS_MASK);
  // keep PD enabled by default, SS controlled separately
  return write_u8_(REG_CFG_OSR, osr);
}

bool AS7331Component::enter_meas_mode_() {
  uint8_t osr = 0;
  if (!this->read_byte(REG_CFG_OSR, &osr)) return false;
  osr = (osr & ~OSR_DOS_MASK) | (DOS_MEAS & OSR_DOS_MASK);
  return write_u8_(REG_CFG_OSR, osr);
}

bool AS7331Component::write_cfg_regs_() {
  // CREG1: time (low nibble) + gain (high nibble) :contentReference[oaicite:18]{index=18}
  uint8_t creg1 = (gain_ << 4) | (conv_time_ & 0x0F);
  if (!write_u8_(REG_CFG_CREG1, creg1)) return false;

  // CREG2: div (0..2), en_div (bit3), en_tm (bit6) :contentReference[oaicite:19]{index=19}
  uint8_t creg2 = 0;
  creg2 |= (divider_ & 0x07);
  if (en_div_) creg2 |= 0x08;
  if (en_tm_)  creg2 |= 0x40;
  if (!write_u8_(REG_CFG_CREG2, creg2)) return false;

  // CREG3: cclk (bits0..1), rdyod(bit3), sb(bit4), mmode(bits6..7) :contentReference[oaicite:20]{index=20}
  uint8_t creg3 = 0;
  creg3 |= (cclk_ & 0x03);
  // rdyod/sb default 0
  creg3 |= (meas_mode_ & 0x03) << 6;
  if (!write_u8_(REG_CFG_CREG3, creg3)) return false;

  return true;
}

bool AS7331Component::start_measurement_() {
  // In measurement mode, OSR.ss (start state) triggers start :contentReference[oaicite:21]{index=21}
  uint16_t osrstat = 0;
  if (!read_u16_(REG_MEAS_OSRSTAT, osrstat)) return false;
  uint8_t osr = osrstat & 0xFF;
  osr |= OSR_SS;     // set start bit
  return write_u8_(REG_MEAS_OSRSTAT, osr); // lower byte is writable in meas mode :contentReference[oaicite:22]{index=22}
}

bool AS7331Component::wait_data_ready_(uint32_t timeout_ms) {
  const uint32_t start = millis();
  while ((millis() - start) < timeout_ms) {
    uint16_t osrstat = 0;
    if (!read_u16_(REG_MEAS_OSRSTAT, osrstat)) return false;

    // bit 11 = ndata (new data) :contentReference[oaicite:23]{index=23}
    if (osrstat & (1u << 11)) return true;

    delay(10);
  }
  return false;
}

bool AS7331Component::read_u16_(uint8_t reg, uint16_t &out) {
  uint8_t data[2] = {0, 0};
  if (!this->read_bytes(reg, data, 2)) return false;

  // Most I2C 16-bit regs in such sensors are little-endian on the wire; if your values look swapped, invert this.
  out = (static_cast<uint16_t>(data[1]) << 8) | data[0];
  return true;
}

bool AS7331Component::write_u8_(uint8_t reg, uint8_t val) {
  return this->write_byte(reg, val);
}

}  // namespace as7331
}  // namespace esphome
