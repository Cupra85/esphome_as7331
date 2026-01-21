#pragma once

#include "esphome/core/component.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace as7331 {

class AS7331Component : public PollingComponent,
                        public i2c::I2CDevice {
 public:
  void setup() override;
  void update() override;
  void dump_config() override;

  // bestehende Setter
  void set_uva_raw(sensor::Sensor *s) { uva_raw_ = s; }
  void set_uvb_raw(sensor::Sensor *s) { uvb_raw_ = s; }
  void set_uvc_raw(sensor::Sensor *s) { uvc_raw_ = s; }

  void set_uva(sensor::Sensor *s) { uva_ = s; }
  void set_uvb(sensor::Sensor *s) { uvb_ = s; }
  void set_uvc(sensor::Sensor *s) { uvc_ = s; }

  void set_uv_index(sensor::Sensor *s) { uv_index_ = s; }

  // --- Erweiterungen ---
  void set_auto_gain(bool v) { auto_gain_ = v; }
  void set_auto_time(bool v) { auto_time_ = v; }

  void set_uva_calibration(float v) { uva_cal_ = v; }
  void set_uvb_calibration(float v) { uvb_cal_ = v; }
  void set_uvc_calibration(float v) { uvc_cal_ = v; }

 protected:
  // bestehend
  uint8_t gain_ = 3;
  uint8_t int_time_ = 4;

  void auto_adjust_(uint16_t uva, uint16_t uvb, uint16_t uvc);
  void write_config_();

  // --- Erweiterungen ---
  bool auto_gain_{true};
  bool auto_time_{true};

  float uva_cal_{1.0f};
  float uvb_cal_{1.0f};
  float uvc_cal_{1.0f};

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
