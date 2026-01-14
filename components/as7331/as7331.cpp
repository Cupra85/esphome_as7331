#include "as7331.h"
#include "esphome/core/log.h"
#include <algorithm>

namespace esphome {
namespace as7331 {

static const char *TAG = "as7331";

/* ================= Register ================= */
static constexpr uint8_t REG_OSR   = 0x00;
static constexpr uint8_t REG_CREG1 = 0x06;
static constexpr uint8_t REG_CREG2 = 0x07;
static constexpr uint8_t REG_CREG3 = 0x08;
static constexpr uint8_t REG_BREAK = 0x09;

/* IMPORTANT: TEMP register block */
static constexpr uint8_t REG_TEMP  = 0x0A;  // TEMP, UVA, UVB, UVC (8 bytes)

/* ================= OSR ================= */
static constexpr uint8_t OSR_CONFIG = 0x02;        // DOS=CONFIG, PD=0
static constexpr uint8_t OSR_MEAS   = 0x03;        // DOS=MEAS,   PD=0
static constexpr uint8_t OSR_START  = 0x83;        // DOS=MEAS, SS=1

/* ================= Status ================= */
static constexpr uint8_t STATUS_NDATA = 0x08;

/* ================= LSB tables (SparkFun / AMS) ================= */
/* unit: µW/cm² per count → multiply by 0.01 for W/m² */

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

/* ================= Setup ================= */
void AS7331Component::setup() {
  write_byte(REG_OSR, OSR_CONFIG);
  delay(3);

  write_byte(REG_CREG1, (gain_ << 4) | int_time_);
  write_byte(REG_CREG2, 0x40);  // SparkFun default
  write_byte(REG_CREG3, 0x00);  // CONT mode
  write_byte(REG_BREAK, 0x19);

  write_byte(REG_OSR, OSR_MEAS);
  delay(3);
  write_byte(REG_OSR, OSR_START);

  ESP_LOGI(TAG, "AS7331 started (CONT, TEMP-register sync read)");
}

/* ================= Update ================= */
void AS7331Component::update() {
  uint8_t osr[2];
  if (!read_bytes(REG_OSR, osr, 2)) return;
  if (!(osr[1] & STATUS_NDATA)) return;

  /* ===== CRITICAL PART ===== */
  uint8_t raw[8];
  if (!read_bytes(REG_TEMP, raw, 8)) return;

  // raw[0..1] = TEMP (ignored)
  uint16_t uva = (raw[3] << 8) | raw[2];
  uint16_t uvb = (raw[5] << 8) | raw[4];
  uint16_t uvc = (raw[7] << 8) | raw[6];

  if (uva_raw_) uva_raw_->publish_state(uva);
  if (uvb_raw_) uvb_raw_->publish_state(uvb);
  if (uvc_raw_) uvc_raw_->publish_state(uvc);

  const float uva_wm2 = uva * LSB_UVA[gain_][int_time_] * 0.01f;
  const float uvb_wm2 = uvb * LSB_UVB[gain_][int_time_] * 0.01f;
  const float uvc_wm2 = uvc * LSB_UVC[gain_][int_time_] * 0.01f;

  if (uva_wm2_) uva_wm2_->publish_state(uva_wm2);
  if (uvb_wm2_) uvb_wm2_->publish_state(uvb_wm2);
  if (uvc_wm2_) uvc_wm2_->publish_state(uvc_wm2);

  const float uv_index =
      (uva_wm2 * 0.0025f) +
      (uvb_wm2 * 0.0100f);

  if (uv_index_) uv_index_->publish_state(uv_index);

  ESP_LOGD(TAG,
           "SYNC RAW A:%u B:%u C:%u | W/m² A:%.6f B:%.6f C:%.6f | UVI %.2f",
           uva, uvb, uvc,
           uva_wm2, uvb_wm2, uvc_wm2,
           uv_index);
}

}  // namespace as7331
}  // namespace esphome
