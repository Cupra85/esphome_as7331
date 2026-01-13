#pragma once

#include "esphome/core/component.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace as7331 {

class AS7331Component : public PollingComponent, public i2c::I2CDevice {
 public:
  void set_gain(uint8_t g) { gain_ = g; }
  void set_int_time(uint8_t t) { int_time_ = t; }

  void set_uva_raw_sensor(sensor::Sensor *s) { uva_raw_ = s; }
  void set_uvb_raw_sensor(sensor::Sensor *s) { uvb_raw_ = s; }
  void set_uvc_raw_sensor(sensor::Sensor *s) { uvc_raw_ = s; }

  void set_uva_wm2_sensor(sensor::Sensor *s) { uva_wm2_ = s; }
  void set_uvb_wm2_sensor(sensor::Sensor *s) { uvb_wm2_ = s; }
  void set_uvc_wm2_sensor(sensor::Sensor *s) { uvc_wm2_ = s; }

  void set_uv_index_sensor(sensor::Sensor *s) { uv_index_ = s; }

  void setup() override;
  void update() override;

 protected:
  uint8_t gain_{3};
  uint8_t int_time_{4};

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
