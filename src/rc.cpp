// Work with the RC receiver (FIXED for ESP32-C3: Serial1 + pin mapping)

#include <Arduino.h>
#include <SBUS.h>
#include "util.h"

// ===================== ESP32-C3 (XIAO) SBUS UART =====================
// ESP32-C3 has Serial and Serial1 only. No Serial2.
#define SBUS_RX_PIN 20   // XIAO D7 = GPIO20  (recommended RX pin for SBUS)
#define SBUS_TX_PIN -1   // not used

// SBUS uses 100000 baud, 8E2 (8 data, even parity, 2 stop bits)
static const uint32_t SBUS_BAUD = 100000;

// Use Serial1 on ESP32-C3
SBUS rc(Serial1);

// =====================================================================

uint16_t channels[16];
float controlTime;

float channelZero[16];
float channelCenter[16];
float channelMax[16];

// Channel indexes (stored as float for params)
float rollChannel = NAN, pitchChannel = NAN, throttleChannel = NAN, yawChannel = NAN, modeChannel = NAN;

void setupRC() {
  print("Setup RC (SBUS on Serial1)\n");

  // IMPORTANT: Configure UART pins + SBUS frame format BEFORE rc.begin()
  Serial1.begin(SBUS_BAUD, SERIAL_8E2, SBUS_RX_PIN, SBUS_TX_PIN);

  rc.begin();
}

bool readRC() {
  if (!rc.read()) return false;

  SBUSData data = rc.data();
  for (int i = 0; i < 16; i++) channels[i] = data.ch[i];

  normalizeRC();
  controlTime = t;
  return true;
}

static inline float mapCentered(uint16_t v, float center, float minv, float maxv) {
  float down = center - minv;
  float up   = maxv - center;
  if (down < 5 || up < 5) return NAN;

  float fv = (float)v;
  if (fv >= center) return  clampf((fv - center) / up,   0.0f, 1.0f);
  else              return -clampf((center - fv) / down, 0.0f, 1.0f);
}

static inline float map01(uint16_t v, float minv, float maxv) {
  float span = maxv - minv;
  if (span < 5) return NAN;
  return clampf(((float)v - minv) / span, 0.0f, 1.0f);
}

void normalizeRC() {
  controlRoll  = (rollChannel >= 0)  ? mapCentered(channels[(int)rollChannel],  channelCenter[(int)rollChannel],  channelZero[(int)rollChannel],  channelMax[(int)rollChannel])  : NAN;
  controlPitch = (pitchChannel >= 0) ? mapCentered(channels[(int)pitchChannel], channelCenter[(int)pitchChannel], channelZero[(int)pitchChannel], channelMax[(int)pitchChannel]) : NAN;
  controlYaw   = (yawChannel >= 0)   ? mapCentered(channels[(int)yawChannel],   channelCenter[(int)yawChannel],   channelZero[(int)yawChannel],   channelMax[(int)yawChannel])   : NAN;

  // Throttle and mode are 0..1
  controlThrottle = (throttleChannel >= 0) ? map01(channels[(int)throttleChannel], channelZero[(int)throttleChannel], channelMax[(int)throttleChannel]) : NAN;
  controlMode     = (modeChannel >= 0)     ? map01(channels[(int)modeChannel],     channelZero[(int)modeChannel],     channelMax[(int)modeChannel])     : NAN;

  // deadzones
  if (isfinite(controlRoll)  && fabsf(controlRoll)  < 0.05f) controlRoll  = 0.0f;
  if (isfinite(controlPitch) && fabsf(controlPitch) < 0.05f) controlPitch = 0.0f;
  if (isfinite(controlYaw)   && fabsf(controlYaw)   < 0.05f) controlYaw   = 0.0f;
}

void calibrateRC() {
  uint16_t zero[16];
  uint16_t center[16];
  uint16_t maxv[16];

  print("1/8 Calibrating RC: put all switches to default positions [3 sec]\n");
  pause(3);

  calibrateRCChannel(NULL, zero,   zero,   "2/8 Move sticks [3 sec]\n");
  calibrateRCChannel(NULL, center, center, "3/8 Center sticks [3 sec]\n");
  calibrateRCChannel(&throttleChannel, zero,   maxv,  "4/8 Throttle min->max [3 sec]\n");
  calibrateRCChannel(&yawChannel,      center, maxv,  "5/8 Yaw min->max [3 sec]\n");
  calibrateRCChannel(&pitchChannel,    zero,   maxv,  "6/8 Pitch min->max [3 sec]\n");
  calibrateRCChannel(&rollChannel,     zero,   maxv,  "7/8 Roll min->max [3 sec]\n");
  calibrateRCChannel(&modeChannel,     zero,   maxv,  "8/8 Mode switch to max [3 sec]\n");

  for (int i = 0; i < 16; i++) {
    channelZero[i]   = zero[i];
    channelCenter[i] = center[i];
    channelMax[i]    = maxv[i];
  }

  printRCCalibration();
}

void calibrateRCChannel(float *channel, uint16_t in[16], uint16_t out[16], const char *str) {
  print("%s", str);
  pause(3);
  for (int i = 0; i < 30; i++) readRC();
  memcpy(out, channels, sizeof(channels));

  if (channel == NULL) return;

  int ch = -1, diff = 0;
  for (int i = 0; i < 16; i++) {
    int d = abs((int)out[i] - (int)in[i]);
    if (d > diff) { ch = i; diff = d; }
  }
  if (ch >= 0 && diff > 10) *channel = (float)ch;
  else *channel = NAN;
}

void printRCCalibration() {
  print("Control   Ch     Zero   Center Max\n");

  auto pr = [&](const char* name, float ch) {
    if (!(ch >= 0)) { print("%-9s%-7g%-7g%-7g%-7g\n", name, ch, NAN, NAN, NAN); return; }
    int i = (int)ch;
    print("%-9s%-7g%-7g%-7g%-7g\n", name, ch, channelZero[i], channelCenter[i], channelMax[i]);
  };

  pr("Roll", rollChannel);
  pr("Pitch", pitchChannel);
  pr("Yaw", yawChannel);
  pr("Throttle", throttleChannel);
  pr("Mode", modeChannel);
}