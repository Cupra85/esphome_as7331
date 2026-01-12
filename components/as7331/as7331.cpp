#include "as7331.h"
#include "esphome/core/log.h"

namespace esphome {
namespace as7331 {

static const char *TAG = "as7331";

/* Laut Datenblatt: Result Register Start */
static const uint8_t REG_RESULT = 0x10;

void AS7331Component::setup() {
  ESP_LOGI(TAG, "AS7331 minimal setup");

  // Minimal: nichts konfigurieren, nichts starten
}

void AS7331Component::update() {
  uint8_t buf[2];

  if (!read_bytes(REG_RESULT, buf, 2)) {
    ESP_LOGW(TAG, "Read failed");
    return;
  }

  uint16_t raw = (uint16_t(buf[0]) << 8) | buf[1];

  ESP_LOGI(TAG, "RAW UVA = %u (0x%02X 0x%02X)", raw, buf[0], buf[1]);

  if (uva_) uva_->publish_state(raw);
}

}  // namespace as7331
}  // namespace esphome
