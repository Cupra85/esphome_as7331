#pragma once

#include "esphome/core/component.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace as7331 {

class AS7331Component : public PollingComponent, public i2c::I2CDevice {
 public:
  void set_gain(uint16_t gain) { gain_ = gain; }
  void set_integration_time(uint16_t time_ms) { integration_time_ms_ = time_ms; }

  void set_uva_sensor(sensor::Sensor *s) { uva_ = s; }
  void set_uvb_sensor(sensor::Sensor *s) { uvb_ = s; }
  void set_uvc_sensor(sensor::Sensor *s) { uvc_ = s; }
  void set_uvi_sensor(sensor::Sensor *s) { uvi_ = s; }

  void set_measurement_enabled(bool enable);

  void setup() override;
  void update() override;

 protected:
  bool configure_();
  bool start_measurement_();
  bool stop_measurement_();
  bool read_raw_(uint16_t &uva, uint16_t &uvb, uint16_t &uvc);

  uint16_t gain_{128};
  uint16_t integration_time_ms_{64};
  bool measuring_{false};

  sensor::Sensor *uva_{nullptr};
  sensor::Sensor *uvb_{nullptr};
  sensor::Sensor *uvc_{nullptr};
  sensor::Sensor *uvi_{nullptr};
};

}  // namespace as7331
}  // namespace esphome
