#pragma once

#include "esphome/core/component.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace as7331 {

class AS7331Component : public esphome::PollingComponent,
                        public esphome::i2c::I2CDevice {
 public:
  void setup() override;
  void update() override;
  void dump_config() override;

  sensor::Sensor *uva_raw{nullptr};
  sensor::Sensor *uvb_raw{nullptr};
  sensor::Sensor *uvc_raw{nullptr};

  sensor::Sensor *uva_irr{nullptr};
  sensor::Sensor *uvb_irr{nullptr};
  sensor::Sensor *uvc_irr{nullptr};

  sensor::Sensor *uv_index{nullptr};

  uint8_t gain = 3;
  uint8_t int_time = 4;
};

}  // namespace as7331
}  // namespace esphome
