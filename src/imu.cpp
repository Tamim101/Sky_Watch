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
// imu_mpu6050.ino — ESP32-WROOM-32 + MPU6050 over I2C
#include <Wire.h>
#include "vector.h"
#include "lpf.h"
#include "util.h"
#define MPU6050_ADDR 0x68
// ESP32-WROOM-32 I2C pins
#define I2C_SDA_PIN 21
#define I2C_SCL_PIN 22
extern Vector acc;
extern Vector gyro;
extern bool landed;
extern float loopRate;
Vector accBias;
Vector accScale(1, 1, 1);
Vector gyroBias;
const float G = 9.80665f;
const float ACC_LSB = 8192.0f; // ±4g
const float GYRO_LSB = 16.4f; // ±2000 dps
const float DEG2RAD = 0.01745329251994f;
static inline void writeReg(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(reg);
  Wire.write(val);
  Wire.endTransmission();
}
static inline void readRegs(uint8_t reg, uint8_t *buf, uint8_t len) {
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU6050_ADDR, len, true);
  for (int i = 0; i < len; i++) buf[i] = Wire.read();
}
void setupIMU() {
  print("Setup MPU-6050 (I2C)\n");
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.setClock(400000); // 400kHz
  delay(100);
  // Wake up
  writeReg(0x6B, 0x00);
  // Gyro ±2000 dps
  writeReg(0x1B, 0x18);
  // Accel ±4g
  writeReg(0x1C, 0x08);
  // DLPF 42 Hz
  writeReg(0x1A, 0x03);
  // Sample rate = 1kHz/(1+0)
  writeReg(0x19, 0x00);
  // WHO_AM_I should read 0x68 for MPU6050
  uint8_t whoami = 0;
  readRegs(0x75, &whoami, 1);
  print("MPU-6050 WHO_AM_I: 0x%02X\n", whoami);
  print("MPU-6050 OK\n");
  landed = true; // Set landed to true always
}
static void calibrateGyroOnce() {
  static Delay landedDelay(2);
  if (!landedDelay.update(landed)) return;
  static LowPassFilter<Vector> gyroCalibrationFilter(0.001f);
  gyroBias = gyroCalibrationFilter.update(gyro);
}
void readIMU() {
  uint8_t buf[14];
  readRegs(0x3B, buf, 14);
  int16_t ax = (buf[0] << 8) | buf[1];
  int16_t ay = (buf[2] << 8) | buf[3];
  int16_t az = (buf[4] << 8) | buf[5];
  int16_t gx = (buf[8] << 8) | buf[9];
  int16_t gy = (buf[10] << 8) | buf[11];
  int16_t gz = (buf[12] << 8) | buf[13];
  // Convert to Flix expected units
  acc.x = (ax / ACC_LSB) * G;
  acc.y = (ay / ACC_LSB) * G;
  acc.z = (az / ACC_LSB) * G;
  gyro.x = (gx / GYRO_LSB) * DEG2RAD;
  gyro.y = (gy / GYRO_LSB) * DEG2RAD;
  gyro.z = (gz / GYRO_LSB) * DEG2RAD;
  calibrateGyroOnce();
  // Apply calibration (accBias in m/s^2, accScale dimensionless)
  acc = (acc - accBias) / accScale;
  gyro = gyro - gyroBias;
  // Orientation correction (adjust if your mounting differs)
  acc = Vector(acc.y, acc.x, -acc.z);
  gyro = Vector(gyro.y, gyro.x, -gyro.z);
  landed = true; // Ensure landed is always true
}
void calibrateAccel() {
  print("Calibrating MPU-6050 accel…\n");
  Vector accMax(-INFINITY, -INFINITY, -INFINITY);
  Vector accMin( INFINITY, INFINITY, INFINITY);
  for (int face = 1; face <= 6; face++) {
    print("%d/6 hold still [8 sec]\n", face);
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
    // Convert average to m/s^2 (IMPORTANT: keep same units as readIMU)
    Vector avg = (sum / samples) / ACC_LSB * G;
    accMax.x = max(accMax.x, avg.x);
    accMax.y = max(accMax.y, avg.y);
    accMax.z = max(accMax.z, avg.z);
    accMin.x = min(accMin.x, avg.x);
    accMin.y = min(accMin.y, avg.y);
    accMin.z = min(accMin.z, avg.z);
  }
  // Bias in m/s^2
  accBias = (accMax + accMin) / 2.0f;
  // Scale should normalize each axis span to 1g
  Vector span = (accMax - accMin) / 2.0f; // ~G on each axis
  accScale = Vector(span.x / G, span.y / G, span.z / G);
  print("Accel calibration done!\n");
  printIMUCalibration();
}
void printIMUInfo() {
  // WHO_AM_I register for MPU6050 = 0x75, expected value 0x68
  uint8_t whoami = 0;
  readRegs(0x75, &whoami, 1);
  print("status: OK\n");
  print("model: MPU-6050\n");
  print("who am I: 0x%02X\n", whoami);
  print("rate: %.0f\n", loopRate);
  print("gyro: %f %f %f\n", gyro.x, gyro.y, gyro.z);
  print("acc: %f %f %f\n", acc.x, acc.y, acc.z);
}
void printIMUCalibration() {
  print("gyro bias: %f %f %f\n", gyroBias.x, gyroBias.y, gyroBias.z);
  print("accel bias: %f %f %f\n", accBias.x, accBias.y, accBias.z);
  print("accel scale:%f %f %f\n", accScale.x, accScale.y, accScale.z);
}
void printIMUCalibration() {
  print("gyro bias: %f %f %f\n", gyroBias.x, gyroBias.y, gyroBias.z);
  print("accel bias: %f %f %f\n", accBias.x, accBias.y, accBias.z);
  print("accel scale:%f %f %f\n", accScale.x, accScale.y, accScale.z);
}