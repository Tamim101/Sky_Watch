//// imu_mpu6050.ino — ESP32-WROOM-32 + MPU6050 over I2C
//
//#include <Wire.h>
//#include "vector.h"
//#include "lpf.h"
//#include "util.h"
//
//#define MPU6050_ADDR 0x68
//
//// ESP32-WROOM-32 I2C pins
//#define I2C_SDA_PIN 21
//#define I2C_SCL_PIN 22
//
//extern Vector acc;
//extern Vector gyro;
//extern bool  landed;
//extern float loopRate;
//
//Vector accBias;
//Vector accScale(1, 1, 1);
//Vector gyroBias;
//
//const float G        = 9.80665f;
//const float ACC_LSB  = 8192.0f;   // ±4g
//const float GYRO_LSB = 16.4f;     // ±2000 dps
//const float DEG2RAD  = 0.01745329251994f;
//
//static inline void writeReg(uint8_t reg, uint8_t val) {
//  Wire.beginTransmission(MPU6050_ADDR);
//  Wire.write(reg);
//  Wire.write(val);
//  Wire.endTransmission();
//}
//
//static inline void readRegs(uint8_t reg, uint8_t *buf, uint8_t len) {
//  Wire.beginTransmission(MPU6050_ADDR);
//  Wire.write(reg);
//  Wire.endTransmission(false);
//  Wire.requestFrom(MPU6050_ADDR, len, true);
//  for (int i = 0; i < len; i++) buf[i] = Wire.read();
//}
//
//void setupIMU() {
//  print("Setup MPU-6050 (I2C)\n");
//
//  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
//  Wire.setClock(400000); // 400kHz
//
//  delay(100);
//
//  // Wake up
//  writeReg(0x6B, 0x00);
//
//  // Gyro ±2000 dps
//  writeReg(0x1B, 0x18);
//
//  // Accel ±4g
//  writeReg(0x1C, 0x08);
//
//  // DLPF 42 Hz
//  writeReg(0x1A, 0x03);
//
//  // Sample rate = 1kHz/(1+0)
//  writeReg(0x19, 0x00);
//
//  // WHO_AM_I should read 0x68 for MPU6050
//  uint8_t whoami = 0;
//  readRegs(0x75, &whoami, 1);
//  print("MPU-6050 WHO_AM_I: 0x%02X\n", whoami);
//
//  print("MPU-6050 OK\n");
//}
//
//static void calibrateGyroOnce() {
//  static Delay landedDelay(2);
//  if (!landedDelay.update(landed)) return;
//
//  static LowPassFilter<Vector> gyroCalibrationFilter(0.001f);
//  gyroBias = gyroCalibrationFilter.update(gyro);
//}
//
//void readIMU() {
//  uint8_t buf[14];
//  readRegs(0x3B, buf, 14);
//
//  int16_t ax = (buf[0] << 8) | buf[1];
//  int16_t ay = (buf[2] << 8) | buf[3];
//  int16_t az = (buf[4] << 8) | buf[5];
//  int16_t gx = (buf[8] << 8) | buf[9];
//  int16_t gy = (buf[10] << 8) | buf[11];
//  int16_t gz = (buf[12] << 8) | buf[13];
//
//  // Convert to Flix expected units
//  acc.x = (ax / ACC_LSB) * G;
//  acc.y = (ay / ACC_LSB) * G;
//  acc.z = (az / ACC_LSB) * G;
//
//  gyro.x = (gx / GYRO_LSB) * DEG2RAD;
//  gyro.y = (gy / GYRO_LSB) * DEG2RAD;
//  gyro.z = (gz / GYRO_LSB) * DEG2RAD;
//
//  calibrateGyroOnce();
//
//  // Apply calibration (accBias in m/s^2, accScale dimensionless)
//  acc  = (acc - accBias) / accScale;
//  gyro =  gyro - gyroBias;
//
//  // Orientation correction (adjust if your mounting differs)
//  acc  = Vector(acc.y,  acc.x,  -acc.z);
//  gyro = Vector(gyro.y, gyro.x, -gyro.z);
//}
//
//void calibrateAccel() {
//  print("Calibrating MPU-6050 accel…\n");
//
//  Vector accMax(-INFINITY, -INFINITY, -INFINITY);
//  Vector accMin( INFINITY,  INFINITY,  INFINITY);
//
//  for (int face = 1; face <= 6; face++) {
//    print("%d/6 hold still [8 sec]\n", face);
//    pause(8);
//
//    Vector sum(0, 0, 0);
//    const int samples = 1000;
//
//    for (int i = 0; i < samples; i++) {
//      uint8_t b[6];
//      readRegs(0x3B, b, 6);
//      int16_t rax = (b[0] << 8) | b[1];
//      int16_t ray = (b[2] << 8) | b[3];
//      int16_t raz = (b[4] << 8) | b[5];
//
//      sum.x += rax;
//      sum.y += ray;
//      sum.z += raz;
//    }
//
//    // Convert average to m/s^2 (IMPORTANT: keep same units as readIMU)
//    Vector avg = (sum / samples) / ACC_LSB * G;
//
//    accMax.x = max(accMax.x, avg.x);
//    accMax.y = max(accMax.y, avg.y);
//    accMax.z = max(accMax.z, avg.z);
//
//    accMin.x = min(accMin.x, avg.x);
//    accMin.y = min(accMin.y, avg.y);
//    accMin.z = min(accMin.z, avg.z);
//  }
//
//  // Bias in m/s^2
//  accBias = (accMax + accMin) / 2.0f;
//
//  // Scale should normalize each axis span to 1g
//  Vector span = (accMax - accMin) / 2.0f; // ~G on each axis
//  accScale = Vector(span.x / G, span.y / G, span.z / G);
//
//  print("Accel calibration done!\n");
//  printIMUCalibration();
//}
//void printIMUInfo() {
//  // WHO_AM_I register for MPU6050 = 0x75, expected value 0x68
//  uint8_t whoami = 0;
//  readRegs(0x75, &whoami, 1);
//
//  print("status: OK\n");
//  print("model: MPU-6050\n");
//  print("who am I: 0x%02X\n", whoami);
//  print("rate: %.0f\n", loopRate);
//  print("gyro: %f %f %f\n", gyro.x, gyro.y, gyro.z);
//  print("acc:  %f %f %f\n", acc.x, acc.y, acc.z);
//}
//
//
//void printIMUCalibration() {
//  print("gyro bias:  %f %f %f\n", gyroBias.x, gyroBias.y, gyroBias.z);
//  print("accel bias: %f %f %f\n", accBias.x, accBias.y, accBias.z);
//  print("accel scale:%f %f %f\n", accScale.x, accScale.y, accScale.z);
//}

// imu_mpu6050.ino — ESP32-WROOM-32 + MPU6050/6500 over I2C for Flix

#include <Wire.h>
#include "vector.h"
#include "lpf.h"
#include "util.h"

#define MPU_ADDR 0x68  // I2C address when AD0 is low

// ESP32-WROOM-32 hardware I2C pins
#define I2C_SDA_PIN 21
#define I2C_SCL_PIN 22

// Global IMU vectors defined elsewhere (state.cpp / main)
extern Vector acc;
extern Vector gyro;

// From other modules
extern bool  landed;     // not strictly needed for gyro bias now, but kept for compatibility
extern float loopRate;

// Calibration state
Vector accBias(0, 0, 0);        // m/s²
Vector accScale(1, 1, 1);       // dimensionless
Vector gyroBias(0, 0, 0);       // rad/s

// Conversion constants for current config
// Accel: ±4g  -> 8192 LSB/g
// Gyro:  ±2000 dps -> 16.4 LSB/(deg/s)
const float ACC_LSB  = 8192.0f;
const float GYRO_LSB = 16.4f;
const float DEG2RAD  = 0.01745329251994f; // π/180

// -----------------------------------------------------------------------------
// Low-level helpers
// -----------------------------------------------------------------------------
static inline void writeReg(uint8_t reg, uint8_t val) {
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(reg);
    Wire.write(val);
    Wire.endTransmission();
}

static inline void readRegs(uint8_t reg, uint8_t *buf, uint8_t len) {
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(reg);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_ADDR, len, true);
    for (int i = 0; i < len; i++) {
        buf[i] = Wire.read();
    }
}

// -----------------------------------------------------------------------------
// Setup
// -----------------------------------------------------------------------------
void setupIMU() {
    print("Setup IMU (MPU-6050/6500)\n");

    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    Wire.setClock(400000); // 400 kHz

    delay(100);

    // Reset then wake up
    writeReg(0x6B, 0x80);
    delay(100);
    writeReg(0x6B, 0x00);

    // Gyro ±2000 dps (FS_SEL = 3 -> 0b11 << 3 = 0x18)
    writeReg(0x1B, 0x18);

    // Accel ±4g (AFS_SEL = 1 -> 0b01 << 3 = 0x08)
    writeReg(0x1C, 0x08);

    // DLPF: ~42 Hz bandwidth (CONFIG register)
    writeReg(0x1A, 0x03);

    // Sample rate = 1 kHz / (1 + SMPLRT_DIV)
    // Here SMPLRT_DIV = 0 -> 1 kHz
    writeReg(0x19, 0x00);

    // Check WHO_AM_I (0x68 for 6050, 0x70 for 6500-class clones)
    uint8_t whoami = 0;
    readRegs(0x75, &whoami, 1);

    if (whoami == 0x68) {
        print("IMU WHO_AM_I: 0x%02X (MPU-6050)\n", whoami);
    } else if (whoami == 0x70) {
        print("IMU WHO_AM_I: 0x%02X (MPU-6500 / clone)\n", whoami);
    } else {
        print("IMU WHO_AM_I: 0x%02X (UNEXPECTED!)\n", whoami);
    }

    // Start with identity calibration (safe defaults)
    accBias  = Vector(0, 0, 0);
    accScale = Vector(1, 1, 1);
    gyroBias = Vector(0, 0, 0);

    print("IMU init done.\n");
    printIMUCalibration();
}

// -----------------------------------------------------------------------------
// Gyro bias calibration (one-shot at startup, no 'landed' dependency)
// -----------------------------------------------------------------------------
static void calibrateGyroOnce() {
    // This will learn gyroBias over the first ~2 seconds after power-up,
    // assuming the quad is stationary on the table.

    static bool done      = false;
    static int  samples   = 0;
    static LowPassFilter<Vector> gyroCalibrationFilter(0.01f); // fairly quick

    if (done) {
        return;
    }

    // Feed current gyro into low-pass to estimate bias
    gyroBias = gyroCalibrationFilter.update(gyro);
    samples++;

    // Assuming loopRate ~1000 Hz, 2000 samples ~2 s
    if (samples > 2000) {
        done = true;
        print("Gyro bias locked: %f %f %f\n", gyroBias.x, gyroBias.y, gyroBias.z);
    }
}

// -----------------------------------------------------------------------------
// Read IMU – call once per main loop
// -----------------------------------------------------------------------------
void readIMU() {
    uint8_t buf[14];
    readRegs(0x3B, buf, 14);

    // Raw sensor values
    int16_t ax = (buf[0] << 8) | buf[1];
    int16_t ay = (buf[2] << 8) | buf[3];
    int16_t az = (buf[4] << 8) | buf[5];
    // buf[6], buf[7] = temperature (ignored)
    int16_t gx = (buf[8]  << 8) | buf[9];
    int16_t gy = (buf[10] << 8) | buf[11];
    int16_t gz = (buf[12] << 8) | buf[13];

    // Convert to physical units
    // accel: m/s²
    acc.x = (ax / ACC_LSB) * ONE_G;
    acc.y = (ay / ACC_LSB) * ONE_G;
    acc.z = (az / ACC_LSB) * ONE_G;

    // gyro: rad/s
    gyro.x = (gx / GYRO_LSB) * DEG2RAD;
    gyro.y = (gy / GYRO_LSB) * DEG2RAD;
    gyro.z = (gz / GYRO_LSB) * DEG2RAD;

    // Update gyro bias during the first ~2 seconds
    calibrateGyroOnce();

    // Apply calibration
    // (component-wise operations – see vector.h)
    acc  = (acc - accBias) / accScale;
    gyro =  gyro - gyroBias;

    // Orientation correction: adjust if your IMU board is mounted differently.
    // This mapping matches the Flix convention you were using.
    acc  = Vector(acc.y,  acc.x,  -acc.z);
    gyro = Vector(gyro.y, gyro.x, -gyro.z);
}

// -----------------------------------------------------------------------------
// Accelerometer 6-face calibration (run from CLI)
// -----------------------------------------------------------------------------
void calibrateAccel() {
    print("Calibrating IMU accel (6 faces)…\n");

    Vector accMax(-INFINITY, -INFINITY, -INFINITY);
    Vector accMin( INFINITY,  INFINITY,  INFINITY);

    // Start from identity (avoid carrying any previous bad state)
    accBias  = Vector(0, 0, 0);
    accScale = Vector(1, 1, 1);

    for (int face = 1; face <= 6; face++) {
        print("%d/6: set new face and hold still [8 s]\n", face);
        pause(8);

        Vector sum(0, 0, 0);
        const int samples = 1000;

        for (int i = 0; i < samples; i++) {
            uint8_t b[6];
            readRegs(0x3B, b, 6);
            int16_t rax = (b[0] << 8) | b[1];
            int16_t ray = (b[2] << 8) | b[3];
            int16_t raz = (b[4] << 8) | b[5];

            sum.x += rax;
            sum.y += ray;
            sum.z += raz;
        }

        // Average raw counts -> m/s² (same units as readIMU)
        Vector avg = (sum / samples) / ACC_LSB * ONE_G;

        accMax.x = max(accMax.x, avg.x);
        accMax.y = max(accMax.y, avg.y);
        accMax.z = max(accMax.z, avg.z);

        accMin.x = min(accMin.x, avg.x);
        accMin.y = min(accMin.y, avg.y);
        accMin.z = min(accMin.z, avg.z);
    }

    // Compute span (~ONE_G per axis if you did ±g on each)
    Vector span = (accMax - accMin) / 2.0f;

    // Guard: if span is too small, keep default scale for that axis
    const float MIN_SPAN = ONE_G * 0.3f; // require at least 0.3 g

    auto computeAxis = [](float spanAxis,
                          float maxAxis,
                          float minAxis,
                          float &scaleAxis,
                          float &biasAxis) {
        if (fabsf(spanAxis) > MIN_SPAN) {
            float s = spanAxis / ONE_G;  // expected ≈ 1.0

            // sanity clamp: if scale is crazy, fall back to 1.0
            if (s > 0.5f && s < 2.0f) {
                scaleAxis = s;
                biasAxis  = (maxAxis + minAxis) / 2.0f;
            } else {
                scaleAxis = 1.0f;
                biasAxis  = 0.0f;
            }
        } else {
            // Did not see enough ±g on this axis → keep identity
            scaleAxis = 1.0f;
            biasAxis  = 0.0f;
        }
    };

    computeAxis(span.x, accMax.x, accMin.x, accScale.x, accBias.x);
    computeAxis(span.y, accMax.y, accMin.y, accScale.y, accBias.y);
    computeAxis(span.z, accMax.z, accMin.z, accScale.z, accBias.z);

    print("Accel calibration done.\n");
    printIMUCalibration();
}

// -----------------------------------------------------------------------------
// Debug printing
// -----------------------------------------------------------------------------
void printIMUInfo() {
    uint8_t whoami = 0;
    readRegs(0x75, &whoami, 1);

    if (whoami == 0x68 || whoami == 0x70) {
        print("status: OK\n");
    } else {
        print("status: ERROR (WHO_AM_I=0x%02X)\n", whoami);
    }

    if (whoami == 0x68) {
        print("model: MPU-6050\n");
    } else if (whoami == 0x70) {
        print("model: MPU-6500 / clone\n");
    } else {
        print("model: unknown\n");
    }

    print("who am I: 0x%02X\n", whoami);
    print("rate: %.0f\n", loopRate);
    print("gyro: %f %f %f\n", gyro.x, gyro.y, gyro.z);
    print("acc:  %f %f %f\n", acc.x,  acc.y,  acc.z);
}

void printIMUCalibration() {
    print("gyro bias:  %f %f %f\n",  gyroBias.x,  gyroBias.y,  gyroBias.z);
    print("accel bias: %f %f %f\n",  accBias.x,  accBias.y,  accBias.z);
    print("accel scale:%f %f %f\n", accScale.x, accScale.y, accScale.z);
}


















