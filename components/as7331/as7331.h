#pragma once

#include "esphome/core/component.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace as7331 {

class AS7331Component : public PollingComponent, public i2c::I2CDevice {
 public:
  void set_uva_sensor(sensor::Sensor *s) { uva_ = s; }
  void set_uvb_sensor(sensor::Sensor *s) { uvb_ = s; }
  void set_uvc_sensor(sensor::Sensor *s) { uvc_ = s; }

  void set_uva_irr_sensor(sensor::Sensor *s) { uva_irr_ = s; }
  void set_uvb_irr_sensor(sensor::Sensor *s) { uvb_irr_ = s; }
  void set_uvc_irr_sensor(sensor::Sensor *s) { uvc_irr_ = s; }

  void set_uva_multiplier(float v) { uva_mult_ = v; }
  void set_uvb_multiplier(float v) { uvb_mult_ = v; }
  void set_uvc_multiplier(float v) { uvc_mult_ = v; }

  void set_gain(uint8_t gain) { gain_ = gain; }
  void set_conversion_time(uint8_t t) { conv_time_ = t; }
  void set_cclk(uint8_t c) { cclk_ = c; }
  void set_measurement_mode(uint8_t m) { meas_mode_ = m; }
  void set_enable_divider(bool en) { en_div_ = en; }
  void set_divider(uint8_t d) { divider_ = d; }

  void setup() override;
  void update() override;

 protected:
  // Config regs
  static constexpr uint8_t REG_CFG_OSR   = 0x00;
  static constexpr uint8_t REG_CFG_AGEN  = 0x02;
  static constexpr uint8_t REG_CFG_CREG1 = 0x06;
  static constexpr uint8_t REG_CFG_CREG2 = 0x07;
  static constexpr uint8_t REG_CFG_CREG3 = 0x08;

  // Measurement regs (mode-dependent bank; AS7331 uses overlapping addresses)
  static constexpr uint8_t REG_MEAS_OSRSTAT = 0x00; // 16-bit
  static constexpr uint8_t REG_MEAS_MRES1   = 0x02; // 16-bit
  static constexpr uint8_t REG_MEAS_MRES2   = 0x03; // 16-bit
  static constexpr uint8_t REG_MEAS_MRES3   = 0x04; // 16-bit

  // OSR bits
  static constexpr uint8_t OSR_DOS_MASK = 0x07; // bits 0..2
  static constexpr uint8_t OSR_SS       = 0x80; // start bit

  // Modes
  static constexpr uint8_t DOS_CFG  = 0x02;
  static constexpr uint8_t DOS_MEAS = 0x03;

  bool enter_cfg_mode_();
  bool enter_meas_mode_();
  bool write_cfg_regs_();

  bool start_measurement_();
  bool wait_new_data_(uint32_t timeout_ms);

  bool read_u16_(uint8_t reg, uint16_t &out);
  bool write_u8_(uint8_t reg, uint8_t val);

  sensor::Sensor *uva_{nullptr};
  sensor::Sensor *uvb_{nullptr};
  sensor::Sensor *uvc_{nullptr};

  sensor::Sensor *uva_irr_{nullptr};
  sensor::Sensor *uvb_irr_{nullptr};
  sensor::Sensor *uvc_irr_{nullptr};

  float uva_mult_{0.0f};
  float uvb_mult_{0.0f};
  float uvc_mult_{0.0f};

  uint8_t gain_{10};      // enum for gain=2 in your mapping
  uint8_t conv_time_{6};  // enum for 64ms
  uint8_t cclk_{0};       // 1.024 MHz
  uint8_t meas_mode_{1};  // cmd
  bool en_div_{false};
  uint8_t divider_{0};    // div2
};

}  // namespace as7331
}  // namespace esphome
