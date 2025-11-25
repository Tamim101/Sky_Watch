// Copyright (c) 2023 Oleg Kalachev <okalachev@gmail.com>
// Repository: https://github.com/okalachev/flix

// Work with the IMU sensor

#include <SPI.h>
#include <FlixPeriph.h>
#include <Wire.h>
#include "vector.h"
#include "lpf.h"
#include "util.h"

#define MPU6050_ADDR 0x68

// === ESP32-C3 I2C pins (change if your wiring is different) ===
#define I2C_SDA_PIN 1   // ESP32-C3 IO1  -> MPU6050 SDA
#define I2C_SCL_PIN 0   // ESP32-C3 IO0  -> MPU6050 SCL

extern Vector acc;
extern Vector gyro;


Vector accBias;
Vector accScale(1,1,1);
Vector gyroBias;

void writeReg(uint8_t reg, uint8_t val) {
    Wire.beginTransmission(MPU6050_ADDR);
    Wire.write(reg);
    Wire.write(val);
    Wire.endTransmission();
}

void readRegs(uint8_t reg, uint8_t *buf, uint8_t len) {
    Wire.beginTransmission(MPU6050_ADDR);
    Wire.write(reg);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU6050_ADDR, len, true);
    for (int i = 0; i < len; i++) buf[i] = Wire.read();
}

void setupIMU() {
    print("Setup MPU-6050\n");

    // Use custom I2C pins on ESP32-C3
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    Wire.setClock(400000);   // 400 kHz I2C (optional but good)

    delay(100);

    // Wake up
    writeReg(0x6B, 0x00);

    // Gyro ±2000 dps
    writeReg(0x1B, 0x18);

    // Acc ±4G (same as Flix)
    writeReg(0x1C, 0x08);

    // DLPF: 42 Hz (good for small drones)
    writeReg(0x1A, 0x03);

    // Sample rate = 1 kHz / (1 + 0)
    writeReg(0x19, 0x00);

    print("MPU-6050 OK\n");
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

    // Convert to real units
    acc.x = ax / 8192.0;   // ±4G
    acc.y = ay / 8192.0;
    acc.z = az / 8192.0;

    gyro.x = gx / 16.4;    // ±2000 dps
    gyro.y = gy / 16.4;
    gyro.z = gz / 16.4;

    calibrateGyroOnce();

    // Apply calibration
    acc = (acc - accBias) / accScale;
    gyro = gyro - gyroBias;

    // Orientation fix (same as Flix)
    acc = Vector(acc.y, acc.x, -acc.z);
    gyro = Vector(gyro.y, gyro.x, -gyro.z);
}

void calibrateGyroOnce() {
    static Delay landedDelay(2);
    if (!landedDelay.update(landed)) return;

    static LowPassFilter<Vector> gyroCalibrationFilter(0.001);
    gyroBias = gyroCalibrationFilter.update(gyro);
}

void calibrateAccel() {
    print("Calibrating MPU-6050 accel…\n");

    Vector accMax(-INFINITY, -INFINITY, -INFINITY);
    Vector accMin(INFINITY, INFINITY, INFINITY);

    for (int face = 1; face <= 6; face++) {
        print("%d/6 hold still [8 sec]\n", face);
        pause(8);

        Vector avg(0,0,0);
        const int samples = 1000;

        for (int i = 0; i < samples; i++) {
            uint8_t buf[6];
            readRegs(0x3B, buf, 6);
            int16_t ax = (buf[0] << 8) | buf[1];
            int16_t ay = (buf[2] << 8) | buf[3];
            int16_t az = (buf[4] << 8) | buf[5];
            avg.x += ax;
            avg.y += ay;
            avg.z += az;
        }

        avg = avg / samples / 8192.0; // convert to G

        accMax.x = max(accMax.x, avg.x);
        accMax.y = max(accMax.y, avg.y);
        accMax.z = max(accMax.z, avg.z);
        accMin.x = min(accMin.x, avg.x);
        accMin.y = min(accMin.y, avg.y);
        accMin.z = min(accMin.z, avg.z);
    }

    accScale = (accMax - accMin) / 2.0 / ONE_G;
    accBias = (accMax + accMin) / 2.0;

    print("Accel calibration done!\n");
    printIMUCalibration();
}

void printIMUCalibration() {
    print("gyro bias: %f %f %f\n", gyroBias.x, gyroBias.y, gyroBias.z);
    print("accel bias: %f %f %f\n", accBias.x, accBias.y, accBias.z);
    print("accel scale: %f %f %f\n", accScale.x, accScale.y, accScale.z);
}



void printIMUInfo() {
    // WHO_AM_I register for MPU6050 = 0x75
    uint8_t whoami;
    readRegs(0x75, &whoami, 1);

    print("status: OK\n");
    print("model: MPU-6050\n");
    print("who am I: 0x%02X\n", whoami);
    print("rate: %.0f\n", loopRate);
    print("gyro: %f %f %f\n", gyro.x, gyro.y, gyro.z);
    print("acc: %f %f %f\n", acc.x, acc.y, acc.z);
}
