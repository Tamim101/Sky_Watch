// imu.ino — MPU6050 (GY-521) replacement for Flix MPU9250 code
// FIX: lock gyro bias once (no continuous bias learning), safer DLPF, stable read

#include <Wire.h>
#include <MPU6050.h>     // FlixPeriph driver
#include <FlixPeriph.h>
#include "vector.h"
#include "lpf.h"
#include "util.h"

#define I2C_SDA_PIN 6
#define I2C_SCL_PIN 7

MPU6050 imu(Wire);
Vector imuRotation(0, 0, -PI / 2); // keep your rotation

// ---- Globals expected by Flix ----
Vector gyro;     // rad/s
Vector gyroBias;

Vector acc;      // m/s^2
Vector accBias;
Vector accScale(1, 1, 1);

// ----------------- IMU SETUP -----------------

void setupIMU() {
  print("Setup IMU (MPU6050)\n");

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.setClock(400000);
  delay(50);

  imu.begin();
  configureIMU();

  // reset calibration values
  gyroBias = Vector(0,0,0);
  accBias  = Vector(0,0,0);
  accScale = Vector(1,1,1);
}

void configureIMU() {
  imu.setAccelRange(imu.ACCEL_RANGE_4G);
  imu.setGyroRange(imu.GYRO_RANGE_2000DPS);

  // DLPF_MAX often too noisy for brushed vibration -> can cause estimator jitter/yaw issues
  // If your FlixPeriph does not have specific Hz enums, keep MAX but expect more noise.
  // Try a mid DLPF if available:
  // imu.setDLPF(imu.DLPF_42HZ);
  imu.setDLPF(imu.DLPF_MAX);

  imu.setRate(imu.RATE_1KHZ_APPROX);

  // IMPORTANT: INT not wired -> do NOT use interrupts
  // imu.setupInterrupt();
}

// ----------------- READ IMU -----------------

void readIMU() {
  // Non-blocking poll: prevents freeze without INT
  if (!imu.read()) return;

  imu.getGyro(gyro.x, gyro.y, gyro.z);
  imu.getAccel(acc.x, acc.y, acc.z);

  // Lock gyro bias once (before flight) to prevent yaw drift/spin
  calibrateGyroOnce();

  // apply scale and bias
  acc  = (acc - accBias) / accScale;
  gyro = gyro - gyroBias;

  // rotate to body frame (same logic as Flix)
  Quaternion rotation = Quaternion::fromEuler(imuRotation);
  acc  = Quaternion::rotateVector(acc,  rotation.inversed());
  gyro = Quaternion::rotateVector(gyro, rotation.inversed());
}

// ----------------- GYRO CALIBRATION -----------------
// FIX: lock bias once while disarmed and still.
// Do NOT continuously adapt bias during vibration.

void calibrateGyroOnce() {
  static bool biasLocked = false;
  if (biasLocked) return;

  // Never calibrate while armed
  if (armed) return;

  // Let sensor settle after boot
  static uint32_t t0 = 0;
  if (t0 == 0) t0 = millis();
  if (millis() - t0 < 1000) return;

  // Require stable "landed" for a moment
  static Delay landedDelay(2);
  if (!landedDelay.update(landed)) return;

  const int N = 800;
  Vector sum(0,0,0);
  int n = 0;

  for (int i = 0; i < N; i++) {
    while (!imu.read()) { /* wait for fresh sample */ }

    Vector g;
    imu.getGyro(g.x, g.y, g.z);

    // Reject samples if moving (helps yaw bias accuracy)
    float gmag = sqrtf(g.x*g.x + g.y*g.y + g.z*g.z);
    if (gmag < 0.15f) {
      sum = sum + g;
      n++;
    }
    delay(2);
  }

  if (n > 200) {
    gyroBias = sum / (float)n;
    biasLocked = true;
    print("Gyro bias locked: %f %f %f (n=%d)\n", gyroBias.x, gyroBias.y, gyroBias.z, n);
  } else {
    print("Gyro bias not locked (too much motion). Keep still and try again.\n");
  }
}

// ----------------- ACCEL CALIBRATION -----------------
// MPU6050 has no magnetometer; 6-side accel calibration still works.

void calibrateAccel() {
  print("Calibrating accelerometer (MPU6050)\n");
  imu.setAccelRange(imu.ACCEL_RANGE_2G);

  print("1/6 Place level [8 sec]\n");          pause(8); calibrateAccelOnce();
  print("2/6 Place nose up [8 sec]\n");        pause(8); calibrateAccelOnce();
  print("3/6 Place nose down [8 sec]\n");      pause(8); calibrateAccelOnce();
  print("4/6 Place on right side [8 sec]\n");  pause(8); calibrateAccelOnce();
  print("5/6 Place on left side [8 sec]\n");   pause(8); calibrateAccelOnce();
  print("6/6 Place upside down [8 sec]\n");    pause(8); calibrateAccelOnce();

  printIMUCalibration();
  print("✓ Calibration done!\n");
  configureIMU();
}

void calibrateAccelOnce() {
  const int samples = 1000;
  static Vector accMax(-INFINITY, -INFINITY, -INFINITY);
  static Vector accMin(INFINITY,  INFINITY,  INFINITY);

  Vector sum(0,0,0);

  for (int i = 0; i < samples; i++) {
    while (!imu.read()) { /* spin */ }

    Vector sample;
    imu.getAccel(sample.x, sample.y, sample.z);
    sum = sum + sample;
  }

  acc = sum / (float)samples;

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
  print("gyro bias:  %f %f %f\n", gyroBias.x, gyroBias.y, gyroBias.z);
  print("accel bias: %f %f %f\n", accBias.x, accBias.y, accBias.z);
  print("accel scale:%f %f %f\n", accScale.x, accScale.y, accScale.z);
}

void printIMUInfo() {
  print("who am I: 0x%02X\n", imu.whoAmI());
  print("rate: %.0f\n", loopRate);
  print("gyro: %f %f %f\n", gyro.x, gyro.y, gyro.z);
  print("acc:  %f %f %f\n", acc.x, acc.y, acc.z);

  Vector rawGyro, rawAcc;
  imu.getGyro(rawGyro.x, rawGyro.y, rawGyro.z);
  imu.getAccel(rawAcc.x, rawAcc.y, rawAcc.z);
  print("raw gyro: %f %f %f\n", rawGyro.x, rawGyro.y, rawGyro.z);
  print("raw acc:  %f %f %f\n", rawAcc.x, rawAcc.y, rawAcc.z);
}
