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

  void set_uva_wm2_sensor(sensor::Sensor *s) { uva_wm2_ = s; }
  void set_uvb_wm2_sensor(sensor::Sensor *s) { uvb_wm2_ = s; }
  void set_uvc_wm2_sensor(sensor::Sensor *s) { uvc_wm2_ = s; }

  void set_gain(uint8_t gain) { gain_ = gain & 0x0F; }
  void set_int_time(uint8_t t) { int_time_ = t & 0x0F; }

  void setup() override;
  void update() override;
  void dump_config() override;

 protected:
  uint8_t gain_{10};
  uint8_t int_time_{6};

  sensor::Sensor *uva_{nullptr};
  sensor::Sensor *uvb_{nullptr};
  sensor::Sensor *uvc_{nullptr};

  sensor::Sensor *uva_wm2_{nullptr};
  sensor::Sensor *uvb_wm2_{nullptr};
  sensor::Sensor *uvc_wm2_{nullptr};
};

}  // namespace as7331
}  // namespace esphome
