#include "as7331.h"
#include "esphome/core/log.h"

#include <algorithm>

namespace esphome {
namespace as7331 {

static const char *const TAG = "as7331";

/*
Register map (Measurement state):
0x00: OSR (read gives OSR byte, plus STATUS byte as 2nd byte) :contentReference[oaicite:8]{index=8}
0x02..0x04: MRES1..3 (each 16-bit, LSB first) :contentReference[oaicite:9]{index=9}
Control regs (Configuration state):
0x06: CREG1 (GAIN/TIME), 0x07: CREG2, 0x08: CREG3, 0x09: BREAK :contentReference[oaicite:10]{index=10}
*/

// Registers
static constexpr uint8_t REG_OSR   = 0x00;
static constexpr uint8_t REG_MRES1 = 0x02;  // MRES1(A), then sequential
static constexpr uint8_t REG_CREG1 = 0x06;
static constexpr uint8_t REG_CREG2 = 0x07;
static constexpr uint8_t REG_CREG3 = 0x08;
static constexpr uint8_t REG_BREAK = 0x09;

// OSR values (see datasheet examples) :contentReference[oaicite:11]{index=11}
static constexpr uint8_t OSR_CONFIG_PD_OFF = 0x02;  // DOS=010, PD=0
static constexpr uint8_t OSR_MEAS_PD_OFF   = 0x03;  // DOS=011, PD=0, SS=0
static constexpr uint8_t OSR_MEAS_START    = 0x83;  // DOS=011, PD=0, SS=1

// STATUS bits (2nd byte when reading from 0x00 in measurement state) :contentReference[oaicite:12]{index=12}
static constexpr uint8_t STATUS_NDATA = 0x08;  // bit3
static constexpr uint8_t STATUS_MRESOF = 0x40; // bit6
static constexpr uint8_t STATUS_ADCOF  = 0x20; // bit5

/*
LSB tables:
These are the same 12x8 tables you posted.
Interpretation:
- rows: GAIN code 0..11 (0=2048x ... 11=1x) :contentReference[oaicite:13]{index=13}
- cols: TIME code 0..7 (1..128ms @ fCLK=1.024MHz) :contentReference[oaicite:14]{index=14}
Unit: µW/cm² per count (common in many AS7331 examples). Conversion: 1 µW/cm² = 0.01 W/m².
*/
static const float LSB_UVA[12][8] = {
  {0.046,0.023,0.012,0.006,0.003,0.0015,0.00075,0.00038},
  {0.092,0.046,0.023,0.012,0.006,0.0030,0.00150,0.00075},
  {0.184,0.092,0.046,0.023,0.012,0.0060,0.00300,0.00150},
  {0.368,0.184,0.092,0.046,0.023,0.0120,0.00600,0.00300},
  {0.736,0.368,0.184,0.092,0.046,0.0230,0.01200,0.00600},
  {1.47 ,0.736,0.368,0.184,0.092,0.0460,0.02300,0.01200},
  {2.94 ,1.47 ,0.736,0.368,0.184,0.0920,0.04600,0.02300},
  {5.88 ,2.94 ,1.47 ,0.736,0.368,0.1840,0.09200,0.04600},
  {11.8 ,5.88 ,2.94 ,1.47 ,0.736,0.3680,0.18400,0.09200},
  {23.5 ,11.8 ,5.88 ,2.94 ,1.47 ,0.7360,0.36800,0.18400},
  {47.1 ,23.5 ,11.8 ,5.88 ,2.94 ,1.470 ,0.73600,0.36800},
  {94.1 ,47.1 ,23.5 ,11.8 ,5.88 ,2.940 ,1.47000,0.73600},
};

static const float LSB_UVB[12][8] = {
  {0.052,0.026,0.013,0.0065,0.0033,0.0016,0.00082,0.00041},
  {0.104,0.052,0.026,0.013 ,0.0065,0.0033,0.00165,0.00082},
  {0.208,0.104,0.052,0.026 ,0.013 ,0.0065,0.00330,0.00165},
  {0.416,0.208,0.104,0.052 ,0.026 ,0.0130,0.00650,0.00330},
  {0.832,0.416,0.208,0.104 ,0.052 ,0.0260,0.01300,0.00650},
  {1.66 ,0.832,0.416,0.208 ,0.104 ,0.0520,0.02600,0.01300},
  {3.33 ,1.66 ,0.832,0.416 ,0.208 ,0.1040,0.05200,0.02600},
  {6.66 ,3.33 ,1.66 ,0.832 ,0.416 ,0.2080,0.10400,0.05200},
  {13.3 ,6.66 ,3.33 ,1.66  ,0.832 ,0.4160,0.20800,0.10400},
  {26.6 ,13.3 ,6.66 ,3.33  ,1.66  ,0.8320,0.41600,0.20800},
  {53.2 ,26.6 ,13.3 ,6.66  ,3.33  ,1.660 ,0.83200,0.41600},
  {106. ,53.2 ,26.6 ,13.3  ,6.66  ,3.330 ,1.66000,0.83200},
};

static const float LSB_UVC[12][8] = {
  {0.060,0.030,0.015,0.0075,0.0038,0.0019,0.00094,0.00047},
  {0.120,0.060,0.030,0.015 ,0.0075,0.0038,0.00188,0.00094},
  {0.240,0.120,0.060,0.030 ,0.015 ,0.0075,0.00375,0.00188},
  {0.480,0.240,0.120,0.060 ,0.030 ,0.0150,0.00750,0.00375},
  {0.960,0.480,0.240,0.120 ,0.060 ,0.0300,0.01500,0.00750},
  {1.92 ,0.960,0.480,0.240 ,0.120 ,0.0600,0.03000,0.01500},
  {3.84 ,1.92 ,0.960,0.480 ,0.240 ,0.1200,0.06000,0.03000},
  {7.68 ,3.84 ,1.92 ,0.960 ,0.480 ,0.2400,0.12000,0.06000},
  {15.4 ,7.68 ,3.84 ,1.92  ,0.960 ,0.4800,0.24000,0.12000},
  {30.7 ,15.4 ,7.68 ,3.84  ,1.92  ,0.9600,0.48000,0.24000},
  {61.4 ,30.7 ,15.4 ,7.68  ,3.84  ,1.920 ,0.96000,0.48000},
  {122. ,61.4 ,30.7 ,15.4  ,7.68  ,3.840 ,1.92000,0.96000},
};

void AS7331Component::dump_config() {
  ESP_LOGCONFIG(TAG, "AS7331:");
  LOG_I2C_DEVICE(this);
  ESP_LOGCONFIG(TAG, "  Gain code: %u (0..11, 0=2048x .. 11=1x)", gain_code_);
  ESP_LOGCONFIG(TAG, "  Int time code: %u (0..7, 1..128ms @ 1.024MHz)", int_time_code_);
  ESP_LOGCONFIG(TAG, "  Auto gain: %s", auto_gain_ ? "true" : "false");
  ESP_LOGCONFIG(TAG, "  Auto time: %s", auto_time_ ? "true" : "false");
  ESP_LOGCONFIG(TAG, "  Measurement enabled: %s", measurement_enabled_ ? "true" : "false");
}

void AS7331Component::apply_profile_defaults_() {
  if (profile_ == PROFILE_CUSTOM) return;

  switch (profile_) {
    case PROFILE_INDOOR:
      gain_code_ = 0;      // very sensitive
      int_time_code_ = 7;  // 128ms
      break;
    case PROFILE_OUTDOOR:
      gain_code_ = 6;      // medium
      int_time_code_ = 4;  // 16ms
      break;
    case PROFILE_UV_LAMP:
      gain_code_ = 3;
      int_time_code_ = 2;
      break;
    default:
      break;
  }
}

void AS7331Component::write_config_() {
  // Clamp to supported ranges
  gain_code_ = std::min<uint8_t>(gain_code_, 11);
  int_time_code_ = std::min<uint8_t>(int_time_code_, 7);

  // CREG1: [GAIN(7:4)] [TIME(3:0)] :contentReference[oaicite:15]{index=15}
  const uint8_t creg1 = (gain_code_ << 4) | (int_time_code_ & 0x0F);

  // CREG2: keep simple: EN_TM=1 (default), EN_DIV=0, DIV=0 :contentReference[oaicite:16]{index=16}
  const uint8_t creg2 = 0x40;

  // CREG3: MMODE=00 (CONT), SB=1, RDYOD=0, CCLK=00 (1.024MHz) :contentReference[oaicite:17]{index=17}
  const uint8_t creg3 = 0x10;

  // BREAK: default 0x19 (= 25 * 8µs = 200µs pause) :contentReference[oaicite:18]{index=18}
  const uint8_t brk = 0x19;

  if (!this->write_byte(REG_CREG1, creg1)) ESP_LOGW(TAG, "Failed write CREG1");
  if (!this->write_byte(REG_CREG2, creg2)) ESP_LOGW(TAG, "Failed write CREG2");
  if (!this->write_byte(REG_CREG3, creg3)) ESP_LOGW(TAG, "Failed write CREG3");
  if (!this->write_byte(REG_BREAK, brk))   ESP_LOGW(TAG, "Failed write BREAK");

  ESP_LOGI(TAG, "Config written: CREG1=0x%02X (gain=%u,time=%u) CREG2=0x%02X CREG3=0x%02X BREAK=0x%02X",
           creg1, gain_code_, int_time_code_, creg2, creg3, brk);
}

void AS7331Component::start_continuous_() {
  // Ensure measurement state, then start (OSR=83h) :contentReference[oaicite:19]{index=19}
  this->write_byte(REG_OSR, OSR_MEAS_PD_OFF);
  delay(2);
  this->write_byte(REG_OSR, OSR_MEAS_START);
  delay(2);
}

void AS7331Component::stop_() {
  // Stop conversion but remain in measurement state (OSR=03h)
  this->write_byte(REG_OSR, OSR_MEAS_PD_OFF);
  delay(2);
}

bool AS7331Component::read_status_(uint8_t &status) {
  uint8_t buf[2]{0, 0};
  // In measurement state: reading address 0 returns OSR byte + STATUS byte :contentReference[oaicite:20]{index=20}
  if (!this->read_bytes(REG_OSR, buf, 2)) return false;
  status = buf[1];
  return true;
}

bool AS7331Component::read_results_(uint16_t &uva, uint16_t &uvb, uint16_t &uvc) {
  uint8_t buf[6]{0};
  // Start from 0x02: sequential read gives MRES1..3 LSB-first :contentReference[oaicite:21]{index=21}
  if (!this->read_bytes(REG_MRES1, buf, 6)) return false;

  uva = (uint16_t(buf[1]) << 8) | buf[0];
  uvb = (uint16_t(buf[3]) << 8) | buf[2];
  uvc = (uint16_t(buf[5]) << 8) | buf[4];
  return true;
}

void AS7331Component::setup() {
  // Datasheet: After power-up device is in CONFIG but PowerDown; must clear PD via OSR=02h :contentReference[oaicite:22]{index=22}
  this->write_byte(REG_OSR, OSR_CONFIG_PD_OFF);
  delay(3);

  apply_profile_defaults_();
  write_config_();

  // Switch to measurement state
  this->write_byte(REG_OSR, OSR_MEAS_PD_OFF);
  delay(3);

  if (measurement_enabled_) {
    start_continuous_();
    ESP_LOGI(TAG, "AS7331 started in CONT mode (OSR=0x83).");
  } else {
    stop_();
    ESP_LOGI(TAG, "AS7331 measurement disabled at boot.");
  }
}

void AS7331Component::set_measurement_enabled(bool enabled) {
  measurement_enabled_ = enabled;
  if (enabled) {
    start_continuous_();
    ESP_LOGI(TAG, "Measurement enabled (CONT).");
  } else {
    stop_();
    ESP_LOGI(TAG, "Measurement stopped.");
  }
}

void AS7331Component::auto_adjust_(uint16_t uva, uint16_t uvb, uint16_t uvc) {
  if (!auto_gain_ && !auto_time_) return;

  const uint16_t peak = std::max({uva, uvb, uvc});

  // Simple target window
  const uint16_t HI = 60000;
  const uint16_t LO = 1000;

  bool changed = false;

  if (peak >= HI) {
    // reduce sensitivity: first shorten TIME, then increase gain_code (towards 11 = 1x)
    if (auto_time_ && int_time_code_ > 0) {
      int_time_code_--;
      changed = true;
    } else if (auto_gain_ && gain_code_ < 11) {
      gain_code_++;
      changed = true;
    }
  } else if (peak <= LO) {
    // increase sensitivity: first decrease gain_code (towards 0 = 2048x), then increase TIME
    if (auto_gain_ && gain_code_ > 0) {
      gain_code_--;
      changed = true;
    } else if (auto_time_ && int_time_code_ < 7) {
      int_time_code_++;
      changed = true;
    }
  }

  if (changed) {
    // Go back to configuration, write regs, then resume measurement state and start. (Sauber nach Datasheet)
    this->write_byte(REG_OSR, OSR_CONFIG_PD_OFF);
    delay(3);
    write_config_();
    this->write_byte(REG_OSR, OSR_MEAS_PD_OFF);
    delay(3);
    if (measurement_enabled_) start_continuous_();

    ESP_LOGD(TAG, "Auto-adjust applied: gain=%u time=%u", gain_code_, int_time_code_);
  }
}

float AS7331Component::counts_to_wm2_uva_(uint16_t counts) const {
  const float uw_cm2 = counts * LSB_UVA[gain_code_][int_time_code_];
  return uw_cm2 * 0.01f;  // 1 µW/cm² = 0.01 W/m²
}
float AS7331Component::counts_to_wm2_uvb_(uint16_t counts) const {
  const float uw_cm2 = counts * LSB_UVB[gain_code_][int_time_code_];
  return uw_cm2 * 0.01f;
}
float AS7331Component::counts_to_wm2_uvc_(uint16_t counts) const {
  const float uw_cm2 = counts * LSB_UVC[gain_code_][int_time_code_];
  return uw_cm2 * 0.01f;
}

void AS7331Component::update() {
  if (!measurement_enabled_) return;

  // Read STATUS first: use NDATA to ensure values actually advance :contentReference[oaicite:23]{index=23}
  uint8_t status = 0;
  if (!read_status_(status)) {
    ESP_LOGW(TAG, "STATUS read failed");
    return;
  }

  const bool new_data = (status & STATUS_NDATA) != 0;
  if (!new_data) {
    ESP_LOGD(TAG, "No new data (NDATA=0) – skipping publish");
    return;
  }

  if (status & (STATUS_MRESOF | STATUS_ADCOF)) {
    ESP_LOGW(TAG, "Overflow/overdrive detected (STATUS=0x%02X). Consider reducing sensitivity.", status);
  }

  uint16_t uva = 0, uvb = 0, uvc = 0;
  if (!read_results_(uva, uvb, uvc)) {
    ESP_LOGW(TAG, "MRES read failed");
    return;
  }

  // publish raw
  if (uva_raw_) uva_raw_->publish_state(uva);
  if (uvb_raw_) uvb_raw_->publish_state(uvb);
  if (uvc_raw_) uvc_raw_->publish_state(uvc);

  // auto adjust using *current* raw
  auto_adjust_(uva, uvb, uvc);

  // irradiance
  const float uva_wm2 = counts_to_wm2_uva_(uva);
  const float uvb_wm2 = counts_to_wm2_uvb_(uvb);
  const float uvc_wm2 = counts_to_wm2_uvc_(uvc);

  if (uva_wm2_) uva_wm2_->publish_state(uva_wm2);
  if (uvb_wm2_) uvb_wm2_->publish_state(uvb_wm2);
  if (uvc_wm2_) uvc_wm2_->publish_state(uvc_wm2);

  // UV Index (simple approximation; keeps it non-unknown if values exist)
  // Common simplification: UVI roughly proportional to erythemal irradiance.
  // Here: weight UVB stronger than UVA.
  const float ery_wm2 = (uva_wm2 * 0.0025f) + (uvb_wm2 * 0.0100f);
  const float uvi = ery_wm2 / 0.025f;  // 1 UVI = 0.025 W/m² erythemal
  if (uv_index_) uv_index_->publish_state(std::max(0.0f, uvi));
}

}  // namespace as7331
}  // namespace esphome
