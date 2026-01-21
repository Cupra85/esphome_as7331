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
  // Setter
  void set_uva_raw(sensor::Sensor *s) { uva_raw_ = s; }
  void set_uvb_raw(sensor::Sensor *s) { uvb_raw_ = s; }
  void set_uvc_raw(sensor::Sensor *s) { uvc_raw_ = s; }
@@ -25,21 +25,25 @@

  void set_uv_index_as7331(sensor::Sensor *s) { uv_index_as7331_ = s; }

  // Konfiguration (Defaultwerte)
  uint8_t gain_ = 3;      // sinnvoller Mittelwert
  uint8_t int_time_ = 4; // ca. 16 ms

 protected:
  // bestehend
  uint8_t gain_ = 3;
  uint8_t int_time_ = 4;

  // neu (Auto-Regler)
  void auto_adjust_(uint16_t uva, uint16_t uvb, uint16_t uvc);
  void write_config_();

  sensor::Sensor *uva_raw_{nullptr};
  sensor::Sensor *uvb_raw_{nullptr};
  sensor::Sensor *uvc_raw_{nullptr};

  sensor::Sensor *uva_{nullptr};
  sensor::Sensor *uvb_{nullptr};
  sensor::Sensor *uvc_{nullptr};

  sensor::Sensor *uv_index_as7331_{nullptr};
};

}  // namespace as7331
}  // namespace esphome
