class AS7331Component : public esphome::PollingComponent,
                        public esphome::i2c::I2CDevice {
 public:
  void setup() override;
  void update() override;
  void dump_config() override;

  void set_uva_raw(sensor::Sensor *s) { uva_raw = s; }
  void set_uvb_raw(sensor::Sensor *s) { uvb_raw = s; }
  void set_uvc_raw(sensor::Sensor *s) { uvc_raw = s; }

  void set_uva_irr(sensor::Sensor *s) { uva_irr = s; }
  void set_uvb_irr(sensor::Sensor *s) { uvb_irr = s; }
  void set_uvc_irr(sensor::Sensor *s) { uvc_irr = s; }

  void set_uv_index(sensor::Sensor *s) { uv_index = s; }

 protected:
  sensor::Sensor *uva_raw{nullptr};
  sensor::Sensor *uvb_raw{nullptr};
  sensor::Sensor *uvc_raw{nullptr};

  sensor::Sensor *uva_irr{nullptr};
  sensor::Sensor *uvb_irr{nullptr};
  sensor::Sensor *uvc_irr{nullptr};

  sensor::Sensor *uv_index{nullptr};
};
