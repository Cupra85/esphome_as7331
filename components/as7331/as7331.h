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

  void set_gain(uint8_t gain) { gain_ = gain & 0x0F; }
  void set_int_time(uint8_t t) { int_time_ = t & 0x0F; }

  void setup() override;
  void update() override;
  void dump_config() override;

 protected:
  bool write_reg_(uint8_t reg, uint8_t value);
  bool read_results_(uint16_t &uva, uint16_t &uvb, uint16_t &uvc);

  uint8_t gain_{0x0A};      // default wie datasheet reset (1010b)
  uint8_t int_time_{0x06};  // 64ms (0110b) bei fCLK=1.024MHz

  sensor::Sensor *uva_{nullptr};
  sensor::Sensor *uvb_{nullptr};
  sensor::Sensor *uvc_{nullptr};
};

}  // namespace as7331
}  // namespace esphome
