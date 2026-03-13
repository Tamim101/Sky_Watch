# SkyWatch — Custom ESP32 Quadcopter Drone

![SkyWatch Hero](<images/drone-on-laptop.jpg>)

**SkyWatch** is a fully open-source, WiFi-controlled mini quadcopter built on **ESP32**.  
It is designed for learning and experimentation with **drone stabilization**, **IMU sensor fusion**, **PID control**, **motor testing**, **telemetry streaming**, and **ESP32 firmware development**.

This project is great for anyone who wants to understand how a quadcopter actually works at the firmware and control level, instead of treating it like a black box.

---

## Overview

SkyWatch combines:

- **ESP32-based flight control**
- **IMU-based attitude sensing**
- **PID stabilization for Roll, Pitch, and Yaw**
- **Live telemetry over WiFi / WebSocket**
- **Serial command interface for debugging and testing**
- **Motor test, calibration, reset, and logging tools**

It is a compact platform for learning how real drones balance, react, and fly.

---

## Features

- Full **PID stabilization**
  - Roll
  - Pitch
  - Yaw
- **IMU calibration** with validity checks
- **Accelerometer + gyroscope** based attitude estimation
- **WiFi client mode**
- **WebSocket telemetry output**
- **Serial commands** for:
  - calibration
  - motor testing
  - logging
  - reset
- **Live serial monitor feedback**
- **Low-pass filtered derivative**
- **Anti-windup PID logic**
- Compact mini-drone / whoop-style build
- Open-source and easy to modify

---

## Why this project matters

SkyWatch is not just a toy drone. It is a practical learning platform for understanding:

- how drones stabilize themselves
- how IMU data is processed in real time
- how PID controllers affect flight behavior
- how ESP32 can be used in robotics and control systems
- how telemetry and debugging tools help tune a flying system

This makes it especially useful for students, embedded developers, robotics learners, and anyone exploring autonomous flight systems.

---

## Hardware

### Main components
- **ESP32 DevKit / NodeMCU**
- **4x coreless motors**
- **ESCs or direct PWM motor control**
- **MPU6050** or similar IMU
- **1S / 2S LiPo battery**
- Lightweight frame (3D-printed, foam, or custom taped frame)
- Status LEDs
  - **Blue** = armed
  - **Red** = error / warning

---

## Software capabilities

SkyWatch firmware includes:

- sensor reading
- IMU calibration
- motor output control
- PID loop execution
- telemetry reporting
- serial command handling
- real-time debugging

The goal is to keep the project simple enough to learn from, while still being real enough to feel like an actual flight controller.

---

## Serial Commands

Depending on your firmware implementation, SkyWatch supports commands such as:

- `calibrate` — calibrate IMU
- `motortest` — test each motor
- `log` — print debug / telemetry data
- `reset` — reset controller state
- `arm` — arm the drone
- `disarm` — disarm the drone

Update this section to exactly match your current firmware commands.

---

## Project Photos

### Components on scale
![Components on scale](<images/components-on-scale.jpg>)

### Full drone build
![Full drone on laptop](<images/drone-on-laptop2.jpg>)

### Close-up with blue LED
![Drone close-up blue LED](<images/drone-closeup-blue.jpg>)

---

## User Build Gallery

These are the custom build images added from your project shots.

### Cinematic SkyWatch shot 1
![SkyWatch User Image 1](<images/WhatsApp Image 2026-03-02 at 8.53.19 PM (1).jpeg>)

### Cinematic SkyWatch shot 2
![SkyWatch User Image 2](<images/WhatsApp Image 2026-03-02 at 8.53.18 PM (2).jpeg>)

### Workbench build
![SkyWatch Workbench Build](<images/Screenshot From 2025-10-06 00-09-11.png>)

### Drone with red LED
![SkyWatch Red LED](<images/WhatsApp Image 2026-03-02 at 8.53.18 PM (1).jpeg>)

### Full setup with multimeter
![SkyWatch Full Setup](<images/WhatsApp Image 2026-03-02 at 8.53.18 PM (2).jpeg>)

---

## Folder Structure

Example project structure:

```text
SkyWatch/
├── README.md
├── imu.ino
├── pid.ino
├── motors.ino
├── telemetry.ino
├── commands.ino
├── main.ino
└── images/
    ├── drone-on-laptop.jpg
    ├── drone-on-laptop2.jpg
    ├── drone-closeup-blue.jpg
    ├── components-on-scale.jpg
    ├── WhatsApp Image 2026-03-02 at 8.53.19 PM (1).jpeg
    ├── WhatsApp Image 2026-03-02 at 8.53.18 PM (2).jpeg
    ├── WhatsApp Image 2026-03-02 at 8.53.18 PM (1).jpeg
    └── Screenshot From 2025-10-06 00-09-11.png