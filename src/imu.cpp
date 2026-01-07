// Work with the IMU sensor (XIAO ESP32-C3 + MPU9250 SPI)

#include <SPI.h>
#include <FlixPeriph.h>
#include "vector.h"
#include "lpf.h"
#include "util.h"

// ===== XIAO ESP32-C3 SPI PINS =====
#define IMU_CS_PIN   20   // D7  (GPIO20)
#define IMU_SCK_PIN   8   // D8  (GPIO8)
#define IMU_MISO_PIN  9   // D9  (GPIO9)
#define IMU_MOSI_PIN 10   // D10 (GPIO10)

MPU9250 imu(SPI);
Vector imuRotation(0, 0, -PI / 2);

Vector gyro;
Vector gyroBias;

Vector acc;
Vector accBias;
Vector accScale(1, 1, 1);

static void setupIMUSpiPins() {
  pinMode(IMU_CS_PIN, OUTPUT);
  digitalWrite(IMU_CS_PIN, HIGH);

  // Force SPI pins on ESP32-C3 (do NOT rely on defaults)
  SPI.begin(IMU_SCK_PIN, IMU_MISO_PIN, IMU_MOSI_PIN, IMU_CS_PIN);
}

void setupIMU() {
  print("Setup IMU\n");

  setupIMUSpiPins();

  // --------- Choose ONE begin() option ----------
  // Option A (many libs): begin(csPin)
  // imu.begin(IMU_CS_PIN);

  // Option B (some libs): begin() uses default CS internally
  // In that case, it often expects SS pin to be configured.
  // Try this if Option A doesn't compile:
  imu.begin();
  // ---------------------------------------------

  configureIMU();
}

void configureIMU() {
  imu.setAccelRange(imu.ACCEL_RANGE_4G);
  imu.setGyroRange(imu.GYRO_RANGE_2000DPS);
  imu.setDLPF(imu.DLPF_MAX);
  imu.setRate(imu.RATE_1KHZ_APPROX);
  imu.setupInterrupt();
}

void readIMU() {
  imu.waitForData();
  imu.getGyro(gyro.x, gyro.y, gyro.z);
  imu.getAccel(acc.x, acc.y, acc.z);

  calibrateGyroOnce();

  acc = (acc - accBias) / accScale;
  gyro = gyro - gyroBias;

  Quaternion rotation = Quaternion::fromEuler(imuRotation);
  acc  = Quaternion::rotateVector(acc,  rotation.inversed());
  gyro = Quaternion::rotateVector(gyro, rotation.inversed());
}

void calibrateGyroOnce() {
  static Delay landedDelay(2);
  if (!landedDelay.update(landed)) return;

  static LowPassFilter<Vector> gyroBiasFilter(0.001);
  gyroBias = gyroBiasFilter.update(gyro);
}

void calibrateAccel() {
  print("Calibrating accelerometer\n");
  imu.setAccelRange(imu.ACCEL_RANGE_2G);

  print("1/6 Place level [8 sec]\n");      pause(8); calibrateAccelOnce();
  print("2/6 Place nose up [8 sec]\n");    pause(8); calibrateAccelOnce();
  print("3/6 Place nose down [8 sec]\n");  pause(8); calibrateAccelOnce();
  print("4/6 Place on right side [8 sec]\n"); pause(8); calibrateAccelOnce();
  print("5/6 Place on left side [8 sec]\n");  pause(8); calibrateAccelOnce();
  print("6/6 Place upside down [8 sec]\n");   pause(8); calibrateAccelOnce();

  printIMUCalibration();
  print("✓ Calibration done!\n");
  configureIMU();
}

void calibrateAccelOnce() {
  const int samples = 1000;
  static Vector accMax(-INFINITY, -INFINITY, -INFINITY);
  static Vector accMin(INFINITY, INFINITY, INFINITY);

  acc = Vector(0, 0, 0);
  for (int i = 0; i < samples; i++) {
    imu.waitForData();
    Vector sample;
    imu.getAccel(sample.x, sample.y, sample.z);
    acc = acc + sample;
  }
  acc = acc / samples;

  if (acc.x > accMax.x) accMax.x = acc.x;
  if (acc.y > accMax.y) accMax.y = acc.y;
  if (acc.z > accMax.z) accMax.z = acc.z;
  if (acc.x < accMin.x) accMin.x = acc.x;
  if (acc.y < accMin.y) accMin.y = acc.y;
  if (acc.z < accMin.z) accMin.z = acc.z;

  accScale = (accMax - accMin) / 2 / ONE_G;
  accBias  = (accMax + accMin) / 2;
}

void printIMUCalibration() {
  print("gyro bias: %f %f %f\n", gyroBias.x, gyroBias.y, gyroBias.z);
  print("accel bias: %f %f %f\n", accBias.x, accBias.y, accBias.z);
  print("accel scale: %f %f %f\n", accScale.x, accScale.y, accScale.z);
}

void printIMUInfo() {
  imu.status() ? print("status: ERROR %d\n", imu.status()) : print("status: OK\n");
  print("model: %s\n", imu.getModel());
  print("who am I: 0x%02X\n", imu.whoAmI());
  print("rate: %.0f\n", loopRate);
  print("gyro: %f %f %f\n", rates.x, rates.y, rates.z);
  print("acc: %f %f %f\n", acc.x, acc.y, acc.z);

  imu.waitForData();
  Vector rawGyro, rawAcc;
  imu.getGyro(rawGyro.x, rawGyro.y, rawGyro.z);
  imu.getAccel(rawAcc.x, rawAcc.y, rawAcc.z);
  print("raw gyro: %f %f %f\n", rawGyro.x, rawGyro.y, rawGyro.z);
  print("raw acc: %f %f %f\n", rawAcc.x, rawAcc.y, rawAcc.z);
}
