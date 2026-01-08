
#include <Wire.h>
#include <math.h>
#include "vector.h"
#include "util.h"

#define MPU_ADDR 0x68

// ESP32-WROOM-32 I2C pins
#define I2C_SDA_PIN 6   // D4
#define I2C_SCL_PIN 7   // D5

// Globals from firmware
extern Vector acc;
extern Vector gyro;
extern bool landed;
extern float loopRate;
extern bool armed; // IMPORTANT: stop bias calibration after arming

// ===================== IMU->BODY AXIS MAP =====================
// Flix body convention is FLU: +X forward, +Y left, +Z up.
// Start with this (your previous mapping), then ONLY adjust if imu test shows wrong axes/signs.
// ===================== IMU->BODY AXIS MAP (FIXED) =====================
// Body frame: FLU  (+X forward, +Y left, +Z up)

// Accel mapping
#define IMU_MAP_AX  1   // body X = sensor Y
#define IMU_MAP_AY  0   // body Y = sensor X
#define IMU_MAP_AZ  2   // body Z = sensor Z
#define IMU_SIGN_AX +1
#define IMU_SIGN_AY +1
#define IMU_SIGN_AZ +1   // <<< FIX: DO NOT INVERT Z

// Gyro mapping (must match accel orientation)
#define IMU_MAP_GX  1
#define IMU_MAP_GY  0
#define IMU_MAP_GZ  2
#define IMU_SIGN_GX +1
#define IMU_SIGN_GY +1
#define IMU_SIGN_GZ +1
// =============================================================

// =============================================================

static Vector accBias(0, 0, 0);   // m/s^2
static Vector accScale(1, 1, 1);  // dimensionless
static Vector gyroBias(0, 0, 0);  // rad/s

static float ACC_LSB  = 8192.0f;
static float GYRO_LSB = 16.4f;
static const float DEG2RAD = 0.01745329251994f;

// Gyro bias calibration
static const float GYRO_STILL_MAX   = 0.05f; // FIX: tighter still threshold
static const int   GYRO_CAL_SAMPLES = 700;

// Landed detection
static const float LANDED_G_MIN     = 8.5f;
static const float LANDED_G_MAX     = 13.5f;
static const float LANDED_GYRO_MAX  = 1.0f;

// Gravity auto-scale
static bool   gravityScaleDone = false;
static int    gravitySamples   = 0;
static Vector gravitySum(0,0,0);
static const int   GRAVITY_SAMPLES    = 700;
static const float GRAVITY_STILL_MAX  = 0.30f;

// ---------------- I2C helpers ----------------
static inline void writeReg(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  Wire.write(val);
  Wire.endTransmission();
}

static inline uint8_t readReg(uint8_t reg) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, (uint8_t)1, (uint8_t)true);
  return Wire.read();
}

static inline void readRegs(uint8_t reg, uint8_t *buf, uint8_t len) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, len, (uint8_t)true);
  for (int i = 0; i < len; i++) buf[i] = Wire.read();
}

// ---------------- Readback config -> LSB constants ----------------
static void updateScaleFromRegisters() {
  uint8_t aconf = readReg(0x1C);
  uint8_t afs = (aconf >> 3) & 0x03;
  switch (afs) {
    case 0: ACC_LSB = 16384.0f; break;
    case 1: ACC_LSB = 8192.0f;  break;
    case 2: ACC_LSB = 4096.0f;  break;
    case 3: ACC_LSB = 2048.0f;  break;
  }

  uint8_t gconf = readReg(0x1B);
  uint8_t gfs = (gconf >> 3) & 0x03;
  switch (gfs) {
    case 0: GYRO_LSB = 131.0f; break;
    case 1: GYRO_LSB = 65.5f;  break;
    case 2: GYRO_LSB = 32.8f;  break;
    case 3: GYRO_LSB = 16.4f;  break;
  }

  print("MPU readback: GYRO_CONFIG=0x%02X  ACCEL_CONFIG=0x%02X\n", gconf, aconf);
  print("MPU scales:   ACC_LSB=%.1f LSB/g   GYRO_LSB=%.1f LSB/(deg/s)\n", ACC_LSB, GYRO_LSB);

  uint8_t dlpf = readReg(0x1A);
  uint8_t smpl = readReg(0x19);
  print("MPU readback: CONFIG(DLPF)=0x%02X  SMPLRT_DIV=0x%02X\n", dlpf, smpl);
}

// ---------------- Gyro bias calibration ----------------
static void calibrateGyroOnce(const Vector& gyro_raw) {
  static bool done = false;
  static int samples = 0;
  static Vector sum(0,0,0);

  if (done) return;

  float gmag = sqrtf(gyro_raw.x*gyro_raw.x + gyro_raw.y*gyro_raw.y + gyro_raw.z*gyro_raw.z);
  if (gmag < GYRO_STILL_MAX) {
    sum += gyro_raw;
    samples++;
  }

  if (samples >= GYRO_CAL_SAMPLES) {
    gyroBias = sum / (float)samples;
    done = true;
    print("Gyro bias locked: %f %f %f (samples=%d)\n", gyroBias.x, gyroBias.y, gyroBias.z, samples);
  }
}

// ---------------- Gravity auto-scale ----------------
static void autoFixGravityScale(const Vector& acc_raw, const Vector& gyro_raw) {
  if (gravityScaleDone) return;

  float gmag = sqrtf(gyro_raw.x*gyro_raw.x + gyro_raw.y*gyro_raw.y + gyro_raw.z*gyro_raw.z);
  if (gmag > GRAVITY_STILL_MAX) return;

  gravitySum += acc_raw;
  gravitySamples++;

  if (gravitySamples >= GRAVITY_SAMPLES) {
    Vector avg = gravitySum / (float)gravitySamples;
    float amag = sqrtf(avg.x*avg.x + avg.y*avg.y + avg.z*avg.z);

    if (amag > 3.0f) {
      float k = amag / ONE_G;     // measured/expected
      accScale = Vector(k, k, k); // later we divide by accScale
      gravityScaleDone = true;
      print("Auto gravity scale DONE: amag=%.3f ONE_G=%.3f k=%.4f\n", amag, ONE_G, k);
      print("accScale: %f %f %f\n", accScale.x, accScale.y, accScale.z);
    } else {
      gravityScaleDone = true;
      print("Auto gravity scale skipped (amag too small: %.3f)\n", amag);
    }
  }
}

// ============================================================================
// REQUIRED GLOBAL FUNCTIONS
// ============================================================================

void setupIMU() {
  print("Setup IMU (MPU-6050/6500)\n");

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.setClock(400000);
  delay(100);

  // Reset then wake
  writeReg(0x6B, 0x80);
  delay(100);
  writeReg(0x6B, 0x00);

  // Gyro ±2000, accel ±4g, DLPF ~42Hz, sample 1kHz
  writeReg(0x1B, 0x18);
  writeReg(0x1C, 0x08);
  writeReg(0x1A, 0x03);
  writeReg(0x19, 0x00);

  uint8_t whoami = readReg(0x75);
  if (whoami == 0x68) print("IMU WHO_AM_I: 0x%02X (MPU-6050)\n", whoami);
  else if (whoami == 0x70) print("IMU WHO_AM_I: 0x%02X (MPU-6500 / clone)\n", whoami);
  else print("IMU WHO_AM_I: 0x%02X (UNEXPECTED!)\n", whoami);

  updateScaleFromRegisters();

  // reset calibration
  accBias  = Vector(0,0,0);
  accScale = Vector(1,1,1);
  gyroBias = Vector(0,0,0);

  gravityScaleDone = false;
  gravitySamples = 0;
  gravitySum = Vector(0,0,0);

  print("IMU init done.\n");
  printIMUCalibration();
}

void readIMU() {
  uint8_t buf[14];
  readRegs(0x3B, buf, 14);

  int16_t ax = (buf[0] << 8) | buf[1];
  int16_t ay = (buf[2] << 8) | buf[3];
  int16_t az = (buf[4] << 8) | buf[5];
  int16_t gx = (buf[8]  << 8) | buf[9];
  int16_t gy = (buf[10] << 8) | buf[11];
  int16_t gz = (buf[12] << 8) | buf[13];

  // Sensor frame
  Vector acc_raw(
    (ax / ACC_LSB) * ONE_G,
    (ay / ACC_LSB) * ONE_G,
    (az / ACC_LSB) * ONE_G
  );

  Vector gyro_raw(
    (gx / GYRO_LSB) * DEG2RAD,
    (gy / GYRO_LSB) * DEG2RAD,
    (gz / GYRO_LSB) * DEG2RAD
  );

  // Learn gyro bias + gravity scale early (ONLY when disarmed)
  if (!armed) calibrateGyroOnce(gyro_raw);
  if (!gravityScaleDone) autoFixGravityScale(acc_raw, gyro_raw);

  // Apply calibration in sensor frame
  Vector acc_cal  = (acc_raw - accBias) / accScale;
  Vector gyro_cal = (gyro_raw - gyroBias);

  // Remap sensor->body using macros
  float a[3] = { acc_cal.x, acc_cal.y, acc_cal.z };
  float g[3] = { gyro_cal.x, gyro_cal.y, gyro_cal.z };

  acc = Vector(
    IMU_SIGN_AX * a[IMU_MAP_AX],
    IMU_SIGN_AY * a[IMU_MAP_AY],
    IMU_SIGN_AZ * a[IMU_MAP_AZ]
  );

  gyro = Vector(
    IMU_SIGN_GX * g[IMU_MAP_GX],
    IMU_SIGN_GY * g[IMU_MAP_GY],
    IMU_SIGN_GZ * g[IMU_MAP_GZ]
  );

  // Landed detection (in body frame)
  float amag = sqrtf(acc.x*acc.x + acc.y*acc.y + acc.z*acc.z);
  float gmag = sqrtf(gyro.x*gyro.x + gyro.y*gyro.y + gyro.z*gyro.z);
  landed = (amag > LANDED_G_MIN && amag < LANDED_G_MAX && gmag < LANDED_GYRO_MAX);
}

void calibrateAccel() {
  print("Calibrating accel (6 faces)...\n");
  print("Do NOT touch during sampling.\n");

  Vector accMax(-INFINITY, -INFINITY, -INFINITY);
  Vector accMin( INFINITY,  INFINITY,  INFINITY);

  accBias  = Vector(0,0,0);
  accScale = Vector(1,1,1);

  for (int face = 1; face <= 6; face++) {
    print("%d/6: place on a new face and hold still [8 s]\n", face);
    pause(8);

    Vector sum(0,0,0);
    const int samples = 1000;

    for (int i = 0; i < samples; i++) {
      uint8_t b[6];
      readRegs(0x3B, b, 6);
      int16_t rax = (b[0] << 8) | b[1];
      int16_t ray = (b[2] << 8) | b[3];
      int16_t raz = (b[4] << 8) | b[5];

      sum.x += (rax / ACC_LSB) * ONE_G;
      sum.y += (ray / ACC_LSB) * ONE_G;
      sum.z += (raz / ACC_LSB) * ONE_G;
    }

    Vector avg = sum / (float)samples;

    accMax.x = max(accMax.x, avg.x);
    accMax.y = max(accMax.y, avg.y);
    accMax.z = max(accMax.z, avg.z);

    accMin.x = min(accMin.x, avg.x);
    accMin.y = min(accMin.y, avg.y);
    accMin.z = min(accMin.z, avg.z);
  }

  accBias = (accMax + accMin) / 2.0f;

  Vector span = (accMax - accMin) / 2.0f;
  accScale.x = (fabsf(span.x) > 0.1f) ? (span.x / ONE_G) : 1.0f;
  accScale.y = (fabsf(span.y) > 0.1f) ? (span.y / ONE_G) : 1.0f;
  accScale.z = (fabsf(span.z) > 0.1f) ? (span.z / ONE_G) : 1.0f;

  // After manual accel calibration, disable auto gravity scaling
  gravityScaleDone = true;

  print("Accel calibration done.\n");
  printIMUCalibration();
}

void printIMUCalibration() {
  print("gyro bias:  %f %f %f\n", gyroBias.x, gyroBias.y, gyroBias.z);
  print("accel bias: %f %f %f\n", accBias.x, accBias.y, accBias.z);
  print("accel scale:%f %f %f\n", accScale.x, accScale.y, accScale.z);
}

void printIMUInfo() {
  uint8_t whoami = readReg(0x75);

  if (whoami == 0x68 || whoami == 0x70) print("status: OK\n");
  else print("status: ERROR (WHO_AM_I=0x%02X)\n", whoami);

  if (whoami == 0x68) print("model: MPU-6050\n");
  else if (whoami == 0x70) print("model: MPU-6500 / clone\n");
  else print("model: unknown\n");

  print("who am I: 0x%02X\n", whoami);
  print("rate: %.0f\n", loopRate);

  print("gyro: %f %f %f\n", gyro.x, gyro.y, gyro.z);
  print("acc:  %f %f %f\n", acc.x,  acc.y,  acc.z);

  float amag = sqrtf(acc.x*acc.x + acc.y*acc.y + acc.z*acc.z);
  print("acc_mag: %.3f\n", amag);

  print("landed: %d\n", landed ? 1 : 0);
  printIMUCalibration();
}