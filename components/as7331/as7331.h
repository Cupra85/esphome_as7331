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

  // Setter (wichtig für ESPHome Codegen)
  void set_uva_raw(sensor::Sensor *s) { uva_raw_ = s; }
  void set_uvb_raw(sensor::Sensor *s) { uvb_raw_ = s; }
  void set_uvc_raw(sensor::Sensor *s) { uvc_raw_ = s; }

  void set_uva(sensor::Sensor *s) { uva_ = s; }
  void set_uvb(sensor::Sensor *s) { uvb_ = s; }
  void set_uvc(sensor::Sensor *s) { uvc_ = s; }

  void set_uv_index(sensor::Sensor *s) { uv_index_ = s; }

  // Konfiguration (Defaultwerte)
  uint8_t gain_ = 3;      // sinnvoller Mittelwert
  uint8_t int_time_ = 4; // ca. 16 ms

 protected:
  sensor::Sensor *uva_raw_{nullptr};
  sensor::Sensor *uvb_raw_{nullptr};
  sensor::Sensor *uvc_raw_{nullptr};

  sensor::Sensor *uva_{nullptr};
  sensor::Sensor *uvb_{nullptr};
  sensor::Sensor *uvc_{nullptr};

  sensor::Sensor *uv_index_{nullptr};
};

}  // namespace as7331
}  // namespace esphome
