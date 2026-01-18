#include "as7331.h"
#include "esphome/core/log.h"

namespace esphome {
namespace as7331 {

static const char *TAG = "as7331";

/* Registers */
static constexpr uint8_t REG_OSR   = 0x00;
static constexpr uint8_t REG_CREG1 = 0x06;
static constexpr uint8_t REG_CREG3 = 0x08;
static constexpr uint8_t REG_BREAK = 0x09;
static constexpr uint8_t REG_MRES1 = 0x02;

/* OSR values */
static constexpr uint8_t OSR_CONFIG = 0x02;
static constexpr uint8_t OSR_MEAS   = 0x03;
static constexpr uint8_t OSR_START  = 0x83;

/* Typical responsivity @64ms, Datasheet Table 6.1 (counts / µW/cm²) */
static constexpr float RESP_UVA = 0.188f;
static constexpr float RESP_UVB = 0.170f;
static constexpr float RESP_UVC = 0.388f;

void AS7331Component::setup() {
  ESP_LOGI(TAG, "Initializing AS7331");

  // CONFIG mode
  write_byte(REG_OSR, OSR_CONFIG);
  delay(2);

  // Gain & integration time
  write_byte(REG_CREG1, (gain << 4) | (int_time & 0x0F));

  // Clock = 1.024 MHz
  write_byte(REG_CREG3, 0x00);

  // Pause time ~200 µs
  write_byte(REG_BREAK, 0x19);

  // Measurement mode
  write_byte(REG_OSR, OSR_MEAS);
  delay(2);

  // Start CONT
  write_byte(REG_OSR, OSR_START);
}

void AS7331Component::update() {
  uint8_t buf[6];
  if (!read_bytes(REG_MRES1, buf, 6)) {
    ESP_LOGW(TAG, "Read failed");
    return;
  }

  uint16_t uva = (buf[1] << 8) | buf[0];
  uint16_t uvb = (buf[3] << 8) | buf[2];
  uint16_t uvc = (buf[5] << 8) | buf[4];

  if (uva_raw) uva_raw->publish_state(uva);
  if (uvb_raw) uvb_raw->publish_state(uvb);
  if (uvc_raw) uvc_raw->publish_state(uvc);

  // Integration time in seconds
  const float tconv = (1 << int_time) / 1000.0f;

  float uva_w = (uva / RESP_UVA) / tconv * 0.01f;
  float uvb_w = (uvb / RESP_UVB) / tconv * 0.01f;
  float uvc_w = (uvc / RESP_UVC) / tconv * 0.01f;

  if (uva_irr) uva_irr->publish_state(uva_w);
  if (uvb_irr) uvb_irr->publish_state(uvb_w);
  if (uvc_irr) uvc_irr->publish_state(uvc_w);

  // WHO UV Index approximation
  float uvi = uva_w * 0.0025f + uvb_w * 0.010f;
  if (uv_index) uv_index->publish_state(uvi);
}

void AS7331Component::dump_config() {
  ESP_LOGCONFIG(TAG, "AS7331 UV Sensor");
  LOG_I2C_DEVICE(this);
}

}  // namespace as7331
}  // namespace esphome
