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

  // YAML setters
  void set_gain_code(uint8_t gain) { gain_code_ = gain; }
  void set_int_time_code(uint8_t time) { int_time_code_ = time; }
  void set_auto_gain(bool v) { auto_gain_ = v; }
  void set_auto_time(bool v) { auto_time_ = v; }
  void set_profile(AS7331Profile p) { profile_ = p; }

  // Switch control
  void set_measurement_enabled(bool enabled);

  // Sensors (raw counts)
  void set_uva_raw_sensor(sensor::Sensor *s) { uva_raw_ = s; }
  void set_uvb_raw_sensor(sensor::Sensor *s) { uvb_raw_ = s; }
  void set_uvc_raw_sensor(sensor::Sensor *s) { uvc_raw_ = s; }

  // Sensors (irradiance)
  void set_uva_wm2_sensor(sensor::Sensor *s) { uva_wm2_ = s; }
  void set_uvb_wm2_sensor(sensor::Sensor *s) { uvb_wm2_ = s; }
  void set_uvc_wm2_sensor(sensor::Sensor *s) { uvc_wm2_ = s; }

  // UV Index
  void set_uv_index_sensor(sensor::Sensor *s) { uv_index_ = s; }

 private:
  void write_config_();
  void start_continuous_();
  void stop_();
  bool read_status_(uint8_t &status);
  bool read_results_(uint16_t &uva, uint16_t &uvb, uint16_t &uvc);

  void apply_profile_defaults_();
  void auto_adjust_(uint16_t uva, uint16_t uvb, uint16_t uvc);

  float counts_to_wm2_uva_(uint16_t counts) const;
  float counts_to_wm2_uvb_(uint16_t counts) const;
  float counts_to_wm2_uvc_(uint16_t counts) const;

  // YAML-configured parameters (codes as per datasheet)
  uint8_t gain_code_{3};      // 0..11 (0=2048x ... 11=1x) :contentReference[oaicite:6]{index=6}
  uint8_t int_time_code_{4};  // 0..7  (1..128 ms at 1.024MHz) :contentReference[oaicite:7]{index=7}

  bool auto_gain_{false};
  bool auto_time_{false};
  AS7331Profile profile_{PROFILE_CUSTOM};

  bool measurement_enabled_{true};

  // sensor pointers
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
