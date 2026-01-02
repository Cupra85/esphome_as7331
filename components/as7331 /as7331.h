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
  void set_temperature_sensor(sensor::Sensor *s) { temp_ = s; }
  void set_outconv_sensor(sensor::Sensor *s) { outconv_ = s; }

  void set_gain(uint8_t gain) { gain_ = gain; }
  void set_conversion_time(uint8_t t) { conv_time_ = t; }
  void set_cclk(uint8_t c) { cclk_ = c; }
  void set_measurement_mode(uint8_t m) { meas_mode_ = m; }
  void set_enable_divider(bool en) { en_div_ = en; }
  void set_divider(uint8_t d) { divider_ = d; }
  void set_temp_conversion_enabled(bool en) { en_tm_ = en; }

  void setup() override;
  void update() override;

 protected:
  // Register addresses (SparkFun sfDevAS7331.h) :contentReference[oaicite:6]{index=6}
  static constexpr uint8_t REG_CFG_OSR    = 0x00;
  static constexpr uint8_t REG_CFG_AGEN   = 0x02;
  static constexpr uint8_t REG_CFG_CREG1  = 0x06;
  static constexpr uint8_t REG_CFG_CREG2  = 0x07;
  static constexpr uint8_t REG_CFG_CREG3  = 0x08;

  static constexpr uint8_t REG_MEAS_OSRSTAT = 0x00;
  static constexpr uint8_t REG_MEAS_TEMP    = 0x01;
  static constexpr uint8_t REG_MEAS_MRES1   = 0x02;
  static constexpr uint8_t REG_MEAS_MRES2   = 0x03;
  static constexpr uint8_t REG_MEAS_MRES3   = 0x04;
  static constexpr uint8_t REG_MEAS_OUTCONV_L = 0x05;  // lower 16-bit of 24-bit outconv :contentReference[oaicite:7]{index=7}

  // OSR bits
  static constexpr uint8_t OSR_DOS_MASK = 0x07;     // bits 0..2
  static constexpr uint8_t OSR_SW_RES   = 0x08;     // bit 3
  static constexpr uint8_t OSR_PD       = 0x40;     // bit 6
  static constexpr uint8_t OSR_SS       = 0x80;     // bit 7

  // op-modes
  static constexpr uint8_t DOS_CFG  = 0x02;         // DEVICE_MODE_CFG :contentReference[oaicite:8]{index=8}
  static constexpr uint8_t DOS_MEAS = 0x03;         // DEVICE_MODE_MEAS :contentReference[oaicite:9]{index=9}

  bool enter_cfg_mode_();
  bool enter_meas_mode_();
  bool write_cfg_regs_();
  bool start_measurement_();
  bool wait_data_ready_(uint32_t timeout_ms);

  bool read_u16_(uint8_t reg, uint16_t &out);
  bool write_u8_(uint8_t reg, uint8_t val);

  sensor::Sensor *uva_{nullptr};
  sensor::Sensor *uvb_{nullptr};
  sensor::Sensor *uvc_{nullptr};
  sensor::Sensor *temp_{nullptr};
  sensor::Sensor *outconv_{nullptr};

  uint8_t gain_{10};       // default: GAIN_2 (enum 10) :contentReference[oaicite:10]{index=10}
  uint8_t conv_time_{6};   // default: TIME_64MS (enum 6) :contentReference[oaicite:11]{index=11}
  uint8_t cclk_{0};        // default: 1.024MHz (enum 0) :contentReference[oaicite:12]{index=12}
  uint8_t meas_mode_{1};   // default: CMD (enum 1) :contentReference[oaicite:13]{index=13}
  bool en_div_{false};
  uint8_t divider_{0};     // DIV_2
  bool en_tm_{true};
};

}  // namespace as7331
}  // namespace esphome
