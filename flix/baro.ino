// baro.ino - BMP280 altitude estimation (relative)
#include <Wire.h>
#include <Adafruit_BMP280.h>
#include "lpf.h"
#include "util.h"

extern bool motorsActive();

Adafruit_BMP280 bmp;

bool baroOk = false;
float baroPressurePa = NAN;
float baroTempC = NAN;

// altitude (meters), relative to ground reference
float alt = 0.0f;
float altVel = 0.0f;

static float p0 = NAN;                 // baseline pressure
static LowPassFilter<float> altLpf(0.05f);  // smooth alt
static Rate baroRate(50);              // ~50 Hz

void setupBaro() {
  print("Setup Baro\n");
  // Wire.begin() already done in setupIMU() for I2C build
  baroOk = bmp.begin(0x76) || bmp.begin(0x77);
  if (!baroOk) {
    print("BMP280 not detected\n");
    return;
  }

  // Optional: configure sampling (defaults usually OK)
  // bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
  //                 Adafruit_BMP280::SAMPLING_X2,
  //                 Adafruit_BMP280::SAMPLING_X16,
  //                 Adafruit_BMP280::FILTER_X16,
  //                 Adafruit_BMP280::STANDBY_MS_10);

  print("BMP280 OK\n");
}

// Call this after step() so 't' is valid
void updateBaro() {
  if (!baroOk) return;
  if (!baroRate) return;

  float p = bmp.readPressure(); // Pa
  float temp = bmp.readTemperature();

  if (!isfinite(p) || p < 30000 || p > 110000) return;

  baroPressurePa = p;
  baroTempC = temp;

  // Calibrate baseline pressure when motors are OFF (ground)
  static float pSum = 0;
  static int pCount = 0;
  static bool calibrated = false;

  if (!calibrated && !motorsActive()) {
    pSum += p;
    pCount++;
    if (pCount >= 100) { // ~2 seconds at 50 Hz
      p0 = pSum / pCount;
      calibrated = true;
      altLpf.reset();
      alt = 0;
      altVel = 0;
      print("Baro calibrated p0=%g Pa\n", p0);
    }
  }

  if (!isfinite(p0)) return;

  // Relative altitude from pressure ratio
  float altRaw = 44330.0f * (1.0f - powf(p / p0, 0.1903f));
  float altFilt = altLpf.update(altRaw);

  static float prevAlt = NAN;
  static float prevT = NAN;
  if (isfinite(prevT) && (t - prevT) > 0.005f) {
    altVel = (altFilt - prevAlt) / (t - prevT);
  }
  prevAlt = altFilt;
  prevT = t;

  alt = altFilt;
}

void printBaro() {
  print("baro ok: %d\n", baroOk);
  print("p(Pa): %g\n", baroPressurePa);
  print("temp(C): %g\n", baroTempC);
  print("alt(m): %g\n", alt);
  print("vZ(m/s): %g\n", altVel);
}