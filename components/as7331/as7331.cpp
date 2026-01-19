#include "as7331.h"
#include "esphome/core/log.h"

namespace esphome {
namespace as7331 {

static const char *TAG = "as7331";

/* Register */
static constexpr uint8_t REG_OSR   = 0x00;
static constexpr uint8_t REG_MRES1 = 0x02;
static constexpr uint8_t REG_CREG1 = 0x06;
static constexpr uint8_t REG_CREG3 = 0x08;
static constexpr uint8_t REG_BREAK = 0x09;

/* OSR */
static constexpr uint8_t OSR_CONFIG = 0x02;
static constexpr uint8_t OSR_MEAS   = 0x03;
static constexpr uint8_t OSR_START  = 0x83;

/* Responsivity (wie bei dir, unverändert) */
static constexpr float RESP_UVA = 0.188f;
static constexpr float RESP_UVB = 0.170f;
static constexpr float RESP_UVC = 0.388f;

/* Gain Tabelle */
static constexpr float GAIN_TABLE[12] = {
  2048,1024,512,256,128,64,32,16,8,4,2,1

/* Kalibrierfaktoren */
static constexpr float CAL_UVA = 5.00f;  // 1.25 für +25 %
static constexpr float CAL_UVB = 5.00f;  // 1.40 für +40 %
static constexpr float CAL_UVC = 1.25f;  // meist 1.0 lassen
};

void AS7331Component::write_config_() {
  write_byte(REG_CREG1, (gain_ << 4) | (int_time_ & 0x0F));
  ESP_LOGD(TAG, "Config updated → gain=%u int_time=%u", gain_, int_time_);
}

/* ===== AUTO GAIN / TIME ===== */
void AS7331Component::auto_adjust_(uint16_t uva, uint16_t uvb, uint16_t uvc) {
  uint16_t peak = std::max(uva, std::max(uvb, uvc));

  // zu hell
  if (peak > 60000) {
    if (int_time_ > 0) {
      int_time_--;
      write_config_();
    } else if (gain_ < 11) {
      gain_++;
      write_config_();
    }
    return;
  }

  // zu dunkel
  if (peak < 200) {
    if (gain_ > 0) {
      gain_--;
      write_config_();
    } else if (int_time_ < 7) {
      int_time_++;
      write_config_();
    }
  }
}

void AS7331Component::setup() {
  write_byte(REG_OSR, OSR_CONFIG);
  delay(2);

  write_config_();

  write_byte(REG_CREG3, 0x00);
  write_byte(REG_BREAK, 0x19);

  write_byte(REG_OSR, OSR_MEAS);
  delay(2);
  write_byte(REG_OSR, OSR_START);

  ESP_LOGI(TAG, "AS7331 running in CONT mode with Auto-Gain");
}

void AS7331Component::update() {
  uint8_t buf[6];
  if (!read_bytes(REG_MRES1, buf, 6)) return;

  uint16_t uva = (buf[1] << 8) | buf[0];
  uint16_t uvb = (buf[3] << 8) | buf[2];
  uint16_t uvc = (buf[5] << 8) | buf[4];

  if (uva_raw_) uva_raw_->publish_state(uva);
  if (uvb_raw_) uvb_raw_->publish_state(uvb);
  if (uvc_raw_) uvc_raw_->publish_state(uvc);

  auto_adjust_(uva, uvb, uvc);

  float tconv = (1 << int_time_) / 1000.0f;
  float gain_factor = GAIN_TABLE[gain_];

float uva_w = ((uva / (RESP_UVA * gain_factor * tconv)) * 0.01f) * CAL_UVA;
float uvb_w = ((uvb / (RESP_UVB * gain_factor * tconv)) * 0.01f) * CAL_UVB;
float uvc_w = ((uvc / (RESP_UVC * gain_factor * tconv)) * 0.01f) * CAL_UVC;

  if (uva_) uva_->publish_state(uva_w);
  if (uvb_) uvb_->publish_state(uvb_w);
  if (uvc_) uvc_->publish_state(uvc_w);

  // ===== UV Index nach SparkFun / WHO =====
  // UVC wird NICHT berücksichtigt
  float erythem_wm2 =
    (uvb_w) +            // UVB dominiert biologisch
    (uva_w * 0.002f);    // UVA nur minimaler Beitrag

  float uv_index = erythem_wm2 / 0.025f;

  if (uv_index_) {
    uv_index_->publish_state(uv_index);
  }
}
void AS7331Component::dump_config() {
  ESP_LOGCONFIG(TAG, "AS7331 UV Sensor");
  LOG_I2C_DEVICE(this);
  ESP_LOGCONFIG(TAG, "  Auto Gain/Time enabled");
}

}  // namespace as7331
}  // namespace esphome
