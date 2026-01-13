#pragma once

#include "esphome/core/component.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace as7331 {

enum AS7331Profile : uint8_t {
  PROFILE_CUSTOM = 0,
  PROFILE_INDOOR,
  PROFILE_OUTDOOR,
  PROFILE_UV_LAMP,
};

class AS7331Component : public PollingComponent, public i2c::I2CDevice {
 public:
  void setup() override;
  void update() override;
  void dump_config() override;

  // YAML
  void set_gain_code(uint8_t gain) { gain_code_ = gain; }
  void set_int_time_code(uint8_t time) { int_time_code_ = time; }
  void set_auto_gain(bool v) { auto_gain_ = v; }
  void set_auto_time(bool v) { auto_time_ = v; }
  void set_profile(AS7331Profile p) { profile_ = p; }

  void set_measurement_enabled(bool enabled);

  // RAW
  void set_uva_raw_sensor(sensor::Sensor *s) { uva_raw_ = s; }
  void set_uvb_raw_sensor(sensor::Sensor *s) { uvb_raw_ = s; }
  void set_uvc_raw_sensor(sensor::Sensor *s) { uvc_raw_ = s; }

  // W/m²
  void set_uva_wm2_sensor(sensor::Sensor *s) { uva_wm2_ = s; }
  void set_uvb_wm2_sensor(sensor::Sensor *s) { uvb_wm2_ = s; }
  void set_uvc_wm2_sensor(sensor::Sensor *s) { uvc_wm2_ = s; }

  // UV Index
  void set_uv_index_sensor(sensor::Sensor *s) { uv_index_ = s; }

 protected:
  // Measurement-state addresses
  static constexpr uint8_t REG_OSR = 0x00;
  static constexpr uint8_t REG_MRES1 = 0x02;
  static constexpr uint8_t REG_MRES2 = 0x03;
  static constexpr uint8_t REG_MRES3 = 0x04;

  // Configuration registers
  static constexpr uint8_t REG_CREG1 = 0x06;
  static constexpr uint8_t REG_CREG2 = 0x07;
  static constexpr uint8_t REG_CREG3 = 0x08;
  static constexpr uint8_t REG_BREAK = 0x09;

 private:
  void apply_profile_defaults_();
  void write_config_();
  void start_continuous_();
  void stop_();

  bool read_status_(uint8_t &status);
  bool read_u16_(uint8_t reg, uint16_t &out);
  bool read_results_(uint16_t &a, uint16_t &b, uint16_t &c);

  void auto_adjust_(uint16_t a, uint16_t b, uint16_t c);

  void calibrate_dark_offset_();
  uint16_t sub_offset_(uint16_t v, uint16_t off) const { return (v > off) ? (v - off) : 0; }

  float counts_to_wm2_uva_(uint16_t counts) const;
  float counts_to_wm2_uvb_(uint16_t counts) const;
  float counts_to_wm2_uvc_(uint16_t counts) const;

  // YAML params
  uint8_t gain_code_{6};      // 0..11 (dein Wunsch: Zahl)
  uint8_t int_time_code_{4};  // 0..7  (dein Wunsch: Zahl)
  bool auto_gain_{false};
  bool auto_time_{false};
  AS7331Profile profile_{PROFILE_CUSTOM};
  bool measurement_enabled_{true};

  // Dark offsets (Counts)
  bool dark_calibrated_{false};
  uint16_t dark_uva_{0};
  uint16_t dark_uvb_{0};
  uint16_t dark_uvc_{0};

  // Sensors
  sensor::Sensor *uva_raw_{nullptr};
  sensor::Sensor *uvb_raw_{nullptr};
  sensor::Sensor *uvc_raw_{nullptr};

  sensor::Sensor *uva_wm2_{nullptr};
  sensor::Sensor *uvb_wm2_{nullptr};
  sensor::Sensor *uvc_wm2_{nullptr};

  sensor::Sensor *uv_index_{nullptr};
};

}  // namespace as7331
}  // namespace esphome
