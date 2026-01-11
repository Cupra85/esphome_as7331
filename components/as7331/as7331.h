#pragma once

#include "esphome/core/component.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace as7331 {

class AS7331Component : public PollingComponent, public i2c::I2CDevice {
 public:
  void set_gain(uint8_t gain) { gain_ = gain; }
  void set_integration_time(uint8_t time) { integration_time_ = time; }

  void set_uva_sensor(sensor::Sensor *s) { uva_ = s; }
  void set_uvb_sensor(sensor::Sensor *s) { uvb_ = s; }
  void set_uvc_sensor(sensor::Sensor *s) { uvc_ = s; }

  void setup() override;
  void update() override;

 protected:
  void configure_();
  void start_measurement_();

  uint8_t gain_{4};
  uint8_t integration_time_{6};

  sensor::Sensor *uva_{nullptr};
  sensor::Sensor *uvb_{nullptr};
  sensor::Sensor *uvc_{nullptr};
};

}  // namespace as7331
}  // namespace esphome
