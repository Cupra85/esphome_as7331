#pragma once

#include "esphome/core/component.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace as7331 {

class AS7331Component : public esphome::PollingComponent,
                        public esphome::i2c::I2CDevice {
 public:
  // PollingComponent constructor (z. B. 10 s Update-Intervall)
  AS7331Component() : PollingComponent(10000) {}

  void setup() override;
  void update() override;

  // Raw sensors
  void set_uva_raw_sensor(sensor::Sensor *s) { uva_raw_ = s; }
  void set_uvb_raw_sensor(sensor::Sensor *s) { uvb_raw_ = s; }
  void set_uvc_raw_sensor(sensor::Sensor *s) { uvc_raw_ = s; }

  // Irradiance sensors
  void set_uva_sensor(sensor::Sensor *s) { uva_wm2_ = s; }
  void set_uvb_sensor(sensor::Sensor *s) { uvb_wm2_ = s; }
  void set_uvc_sensor(sensor::Sensor *s) { uvc_wm2_ = s; }

  // UV Index
  void set_uv_index_sensor(sensor::Sensor *s) { uv_index_ = s; }

 protected:
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
