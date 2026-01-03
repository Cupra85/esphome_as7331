#pragma once

#include "esphome/core/component.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace as7331 {

class AS7331Component : public PollingComponent, public i2c::I2CDevice {
 public:
  void set_uva_raw_sensor(sensor::Sensor *s) { uva_raw_ = s; }
  void set_uvb_raw_sensor(sensor::Sensor *s) { uvb_raw_ = s; }
  void set_uvc_raw_sensor(sensor::Sensor *s) { uvc_raw_ = s; }

  void set_uva_sensor(sensor::Sensor *s) { uva_ = s; }
  void set_uvb_sensor(sensor::Sensor *s) { uvb_ = s; }
  void set_uvc_sensor(sensor::Sensor *s) { uvc_ = s; }

  void set_uv_index_sensor(sensor::Sensor *s) { uv_index_ = s; }

  void update() override;

 protected:
  sensor::Sensor *uva_raw_{nullptr};
  sensor::Sensor *uvb_raw_{nullptr};
  sensor::Sensor *uvc_raw_{nullptr};

  sensor::Sensor *uva_{nullptr};
  sensor::Sensor *uvb_{nullptr};
  sensor::Sensor *uvc_{nullptr};

  sensor::Sensor *uv_index_{nullptr};

  uint16_t read_uva();
  uint16_t read_uvb();
  uint16_t read_uvc();
};

}  // namespace as7331
}  // namespace esphome
