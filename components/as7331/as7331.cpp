#include "as7331.h"
#include "esphome/core/log.h"
#include <algorithm>

namespace esphome {
namespace as7331 {

static const char *TAG = "as7331";

/* ================= Register ================= */
static const uint8_t REG_OSR   = 0x00;
static const uint8_t REG_CREG1 = 0x06;
static const uint8_t REG_CREG2 = 0x07;
static const uint8_t REG_CREG3 = 0x08;
static const uint8_t REG_BREAK = 0x09;
static const uint8_t REG_MRES1 = 0x02;

/* ================= OSR bits ================= */
static const uint8_t OSR_SS = 0x80;
static const uint8_t DOS_CONFIG = 0x02;
static const uint8_t DOS_MEAS   = 0x03;

/* =========================================================
 * LSB tables (nW/cm² per count)
 * Source: AMS AS7331 datasheet
 * Clock: 1.024 MHz
 * Dimensions: [gain][integration_time]
 * gain: 0..11
 * int_time: 0..7 (1..128 ms)
 * ========================================================= */

/* === UVA === */
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

/* === UVB === */
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

/* === UVC === */
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

/* =========================================================
 * Channel entmisching matrix (inverse)
 * Empirisch stabil, SparkFun/AMS-orientiert
 * ========================================================= */
static constexpr float M_INV[3][3] = {
  {  1.30f, -0.30f, -0.05f },   // UVA
  { -0.25f,  1.35f, -0.10f },   // UVB
  { -0.05f, -0.20f,  1.25f }    // UVC
};

/* ================= Setup ================= */
void AS7331Component::setup() {
  ESP_LOGI(TAG, "AS7331 init (CONT, CCLK=1.024 MHz)");

  write_byte(REG_OSR, DOS_CONFIG);
  delay(2);

  write_byte(REG_CREG1, (gain_ << 4) | int_time_);
  write_byte(REG_CREG2, 0x40);
  write_byte(REG_CREG3, 0x00);
  write_byte(REG_BREAK, 0x19);

  write_byte(REG_OSR, DOS_MEAS);
  delay(2);
  write_byte(REG_OSR, OSR_SS | DOS_MEAS);

  ESP_LOGI(TAG, "AS7331 running");
}

/* ================= Update ================= */
void AS7331Component::update() {
  uint8_t buf[6];
  if (!read_bytes(REG_MRES1, buf, 6)) return;

  uint16_t uva_raw = (buf[1] << 8) | buf[0];
  uint16_t uvb_raw = (buf[3] << 8) | buf[2];
  uint16_t uvc_raw = (buf[5] << 8) | buf[4];

  if (uva_) uva_->publish_state(uva_raw);
  if (uvb_) uvb_->publish_state(uvb_raw);
  if (uvc_) uvc_->publish_state(uvc_raw);

  /* === radiometrische Irradiance (überlappend) === */
  float uva_m = uva_raw * LSB_UVA[gain_][int_time_] * 1e-5f;
  float uvb_m = uvb_raw * LSB_UVB[gain_][int_time_] * 1e-5f;
  float uvc_m = uvc_raw * LSB_UVC[gain_][int_time_] * 1e-5f;

  /* === Entmischung === */
  float uva_true =
    M_INV[0][0]*uva_m + M_INV[0][1]*uvb_m + M_INV[0][2]*uvc_m;

  float uvb_true =
    M_INV[1][0]*uva_m + M_INV[1][1]*uvb_m + M_INV[1][2]*uvc_m;

  float uvc_true =
    M_INV[2][0]*uva_m + M_INV[2][1]*uvb_m + M_INV[2][2]*uvc_m;

  uva_true = std::max(0.0f, uva_true);
  uvb_true = std::max(0.0f, uvb_true);
  uvc_true = std::max(0.0f, uvc_true);

  if (uva_wm2_) uva_wm2_->publish_state(uva_true);
  if (uvb_wm2_) uvb_wm2_->publish_state(uvb_true);
  if (uvc_wm2_) uvc_wm2_->publish_state(uvc_true);

  ESP_LOGD(TAG,
           "RAW %u %u %u | UVA %.6f UVB %.6f UVC %.6f",
           uva_raw, uvb_raw, uvc_raw,
           uva_true, uvb_true, uvc_true);
}

}  // namespace as7331
}  // namespace esphome
