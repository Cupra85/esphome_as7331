#pragma once

#include "esphome/core/component.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace as7331 {

class AS7331Component : public Component, public i2c::I2CDevice {
 public:
  void set_uva_sensor(sensor::Sensor *s) { uva_ = s; }
  void set_uvb_sensor(sensor::Sensor *s) { uvb_ = s; }
  void set_uvc_sensor(sensor::Sensor *s) { uvc_ = s; }
  void set_uvi_sensor(sensor::Sensor *s) { uvi_ = s; }

  void setup() override;
  void update() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

 protected:
  sensor::Sensor *uva_{nullptr};
  sensor::Sensor *uvb_{nullptr};
  sensor::Sensor *uvc_{nullptr};
  sensor::Sensor *uvi_{nullptr};

  bool read_channels_(float &uva, float &uvb, float &uvc);
};

}  // namespace as7331
}  // namespace esphome
