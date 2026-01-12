#pragma once

#include "esphome/core/component.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace as7331 {

class AS7331Component : public PollingComponent, public i2c::I2CDevice {
 public:
  void set_uva_sensor(sensor::Sensor *s) { uva_ = s; }

  void setup() override;
  void update() override;

 protected:
  sensor::Sensor *uva_{nullptr};
};

}  // namespace as7331
}  // namespace esphome
