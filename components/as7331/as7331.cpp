#include "as7331.h"
#include "esphome/core/log.h"

#include <algorithm>

namespace esphome {
namespace as7331 {

static const char *const TAG = "as7331";

// OSR / DOS
static constexpr uint8_t OSR_SS = 0x80;
static constexpr uint8_t DOS_CONFIG = 0x02;
static constexpr uint8_t DOS_MEAS = 0x03;

// OSR “good practice” sequence
static constexpr uint8_t OSR_CONFIG_PD_OFF = 0x02; // DOS=CONFIG, PD=0
static constexpr uint8_t OSR_MEAS_PD_OFF   = 0x03; // DOS=MEAS,   PD=0, SS=0
static constexpr uint8_t OSR_MEAS_START    = 0x83; // DOS=MEAS,   PD=0, SS=1

// STATUS bits (2nd byte when reading REG_OSR in measurement state)
static constexpr uint8_t STATUS_NDATA  = 0x08; // bit3
static constexpr uint8_t STATUS_ADCOF  = 0x20; // bit5
static constexpr uint8_t STATUS_MRESOF = 0x40; // bit6

// LSB tables: identical to your known-good values
// Unit used here: µW/cm² per count
// Conversion: 1 µW/cm² = 0.01 W/m²
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
  ESP_LOGCONFIG(TAG, "  gain: %u (0..11)", gain_code_);
  ESP_LOGCONFIG(TAG, "  int_time: %u (0..7)", int_time_code_);
  ESP_LOGCONFIG(TAG, "  auto_gain: %s", auto_gain_ ? "true" : "false");
  ESP_LOGCONFIG(TAG, "  auto_time: %s", auto_time_ ? "true" : "false");
  ESP_LOGCONFIG(TAG, "  measurement_enabled: %s", measurement_enabled_ ? "true" : "false");
  if (dark_calibrated_) {
    ESP_LOGCONFIG(TAG, "  dark offsets: UVA=%u UVB=%u UVC=%u counts", dark_uva_, dark_uvb_, dark_uvc_);
  }
}

void AS7331Component::apply_profile_defaults_() {
  if (profile_ == PROFILE_CUSTOM) return;

  switch (profile_) {
    case PROFILE_INDOOR:
      gain_code_ = 0;
      int_time_code_ = 7;
      break;
    case PROFILE_OUTDOOR:
      gain_code_ = 6;
      int_time_code_ = 4;
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
  gain_code_ = std::min<uint8_t>(gain_code_, 11);
  int_time_code_ = std::min<uint8_t>(int_time_code_, 7);

  // CREG1: gain(7:4) | time(3:0)
  const uint8_t creg1 = (gain_code_ << 4) | (int_time_code_ & 0x0F);

  // CREG2: simplest stable defaults (like your ref): divider disabled, cclk divider defaults
  // Keep EN_DIV=0, DIV=0; set EN_TM as per typical example -> 0x40
  const uint8_t creg2 = 0x40;

  // CREG3: measurement mode in bits 7:6
  // 00 = CONT
  const uint8_t meas_mode = 0x00;
  const uint8_t cclk = 0x00; // 1.024 MHz
  const uint8_t creg3 = (cclk & 0x03) | ((meas_mode & 0x03) << 6);

  // BREAK default
  const uint8_t brk = 0x19;

  this->write_byte(REG_CREG1, creg1);
  this->write_byte(REG_CREG2, creg2);
  this->write_byte(REG_CREG3, creg3);
  this->write_byte(REG_BREAK, brk);

  ESP_LOGI(TAG, "Config: CREG1=0x%02X (gain=%u,time=%u) CREG2=0x%02X CREG3=0x%02X",
           creg1, gain_code_, int_time_code_, creg2, creg3);
}

void AS7331Component::start_continuous_() {
  this->write_byte(REG_OSR, OSR_MEAS_PD_OFF);
  delay(2);
  this->write_byte(REG_OSR, OSR_MEAS_START);
  delay(2);
}

void AS7331Component::stop_() {
  this->write_byte(REG_OSR, OSR_MEAS_PD_OFF);
  delay(2);
}

bool AS7331Component::read_status_(uint8_t &status) {
  uint8_t buf[2]{0, 0};
  if (!this->read_bytes(REG_OSR, buf, 2)) return false;
  status = buf[1];
  return true;
}

bool AS7331Component::read_u16_(uint8_t reg, uint16_t &out) {
  uint8_t data[2] = {0, 0};
  if (!this->read_bytes(reg, data, 2)) return false;
  out = (static_cast<uint16_t>(data[1]) << 8) | data[0]; // little endian
  return true;
}

bool AS7331Component::read_results_(uint16_t &a, uint16_t &b, uint16_t &c) {
  // IMPORTANT: read each 16-bit register separately (ref project behaviour)
  if (!read_u16_(REG_MRES1, a)) return false;
  if (!read_u16_(REG_MRES2, b)) return false;
  if (!read_u16_(REG_MRES3, c)) return false;
  return true;
}

void AS7331Component::auto_adjust_(uint16_t a, uint16_t b, uint16_t c) {
  if (!auto_gain_ && !auto_time_) return;

  const uint16_t peak = std::max({a, b, c});
  const uint16_t HI = 60000;
  const uint16_t LO = 800;

  bool changed = false;

  if (peak >= HI) {
    if (auto_time_ && int_time_code_ > 0) {
      int_time_code_--;
      changed = true;
    } else if (auto_gain_ && gain_code_ < 11) {
      gain_code_++;
      changed = true;
    }
  } else if (peak <= LO) {
    if (auto_gain_ && gain_code_ > 0) {
      gain_code_--;
      changed = true;
    } else if (auto_time_ && int_time_code_ < 7) {
      int_time_code_++;
      changed = true;
    }
  }

  if (changed) {
    // re-config safely
    this->write_byte(REG_OSR, OSR_CONFIG_PD_OFF);
    delay(3);
    write_config_();
    this->write_byte(REG_OSR, OSR_MEAS_PD_OFF);
    delay(3);
    if (measurement_enabled_) start_continuous_();
    ESP_LOGD(TAG, "Auto-adjust -> gain=%u time=%u", gain_code_, int_time_code_);
  }
}

float AS7331Component::counts_to_wm2_uva_(uint16_t counts) const {
  const float uw_cm2 = counts * LSB_UVA[gain_code_][int_time_code_];
  return uw_cm2 * 0.01f;
}
float AS7331Component::counts_to_wm2_uvb_(uint16_t counts) const {
  const float uw_cm2 = counts * LSB_UVB[gain_code_][int_time_code_];
  return uw_cm2 * 0.01f;
}
float AS7331Component::counts_to_wm2_uvc_(uint16_t counts) const {
  const float uw_cm2 = counts * LSB_UVC[gain_code_][int_time_code_];
  return uw_cm2 * 0.01f;
}

void AS7331Component::calibrate_dark_offset_() {
  // Take a few samples to estimate dark offsets (counts)
  // This reduces "UVC too high" in low-UV environments due to offsets/noise.
  const int N = 8;
  uint32_t sa = 0, sb = 0, sc = 0;
  int got = 0;

  for (int i = 0; i < N; i++) {
    uint8_t status = 0;
    if (!read_status_(status)) continue;
    if ((status & STATUS_NDATA) == 0) { delay(10); continue; }

    uint16_t a=0,b=0,c=0;
    if (!read_results_(a,b,c)) continue;

    sa += a; sb += b; sc += c;
    got++;
    delay(10);
  }

  if (got >= 3) {
    dark_uva_ = sa / got;
    dark_uvb_ = sb / got;
    dark_uvc_ = sc / got;
    dark_calibrated_ = true;
    ESP_LOGI(TAG, "Dark offset calibrated: UVA=%u UVB=%u UVC=%u counts", dark_uva_, dark_uvb_, dark_uvc_);
  } else {
    ESP_LOGW(TAG, "Dark offset calibration skipped (insufficient samples)");
  }
}

void AS7331Component::setup() {
  this->write_byte(REG_OSR, OSR_CONFIG_PD_OFF);
  delay(3);

  apply_profile_defaults_();
  write_config_();

  this->write_byte(REG_OSR, OSR_MEAS_PD_OFF);
  delay(3);

  if (measurement_enabled_) {
    start_continuous_();
    ESP_LOGI(TAG, "AS7331 started in CONT mode.");
  } else {
    stop_();
    ESP_LOGI(TAG, "AS7331 measurement disabled at boot.");
  }

  // Calibrate offsets after measurement start (best effort)
  if (measurement_enabled_) {
    calibrate_dark_offset_();
  }
}

void AS7331Component::set_measurement_enabled(bool enabled) {
  measurement_enabled_ = enabled;
  if (enabled) {
    start_continuous_();
    // refresh offsets when enabling
    calibrate_dark_offset_();
    ESP_LOGI(TAG, "Measurement enabled (CONT).");
  } else {
    stop_();
    ESP_LOGI(TAG, "Measurement stopped.");
  }
}

void AS7331Component::update() {
  if (!measurement_enabled_) return;

  uint8_t status = 0;
  if (!read_status_(status)) {
    ESP_LOGW(TAG, "STATUS read failed");
    return;
  }

  if ((status & STATUS_NDATA) == 0) {
    ESP_LOGD(TAG, "NDATA=0 -> no new data");
    return;
  }

  if (status & (STATUS_ADCOF | STATUS_MRESOF)) {
    ESP_LOGW(TAG, "Overflow flags set: STATUS=0x%02X", status);
  }

  uint16_t a=0,b=0,c=0;
  if (!read_results_(a,b,c)) {
    ESP_LOGW(TAG, "MRES read failed");
    return;
  }

  // apply dark offsets
  uint16_t a0 = dark_calibrated_ ? sub_offset_(a, dark_uva_) : a;
  uint16_t b0 = dark_calibrated_ ? sub_offset_(b, dark_uvb_) : b;
  uint16_t c0 = dark_calibrated_ ? sub_offset_(c, dark_uvc_) : c;

  // publish raw (offset corrected)
  if (uva_raw_) uva_raw_->publish_state(a0);
  if (uvb_raw_) uvb_raw_->publish_state(b0);
  if (uvc_raw_) uvc_raw_->publish_state(c0);

  // auto adjust uses original (not offset) peak to avoid instability at low light
  auto_adjust_(a, b, c);

  // irradiance
  const float uva_wm2 = counts_to_wm2_uva_(a0);
  const float uvb_wm2 = counts_to_wm2_uvb_(b0);
  const float uvc_wm2 = counts_to_wm2_uvc_(c0);

  if (uva_wm2_) uva_wm2_->publish_state(uva_wm2);
  if (uvb_wm2_) uvb_wm2_->publish_state(uvb_wm2);
  if (uvc_wm2_) uvc_wm2_->publish_state(uvc_wm2);

  // UV Index: always publish, never "unknown"
  const float ery_wm2 = (uva_wm2 * 0.0025f) + (uvb_wm2 * 0.0100f);
  const float uvi = std::max(0.0f, ery_wm2 / 0.025f);
  if (uv_index_) uv_index_->publish_state(uvi);

  ESP_LOGD(TAG, "Counts (raw) A:%u B:%u C:%u | (corr) A:%u B:%u C:%u | W/m² U:%.6f B:%.6f C:%.6f | UVI %.2f",
           a,b,c,a0,b0,c0, uva_wm2, uvb_wm2, uvc_wm2, uvi);
}

}  // namespace as7331
}  // namespace esphome
