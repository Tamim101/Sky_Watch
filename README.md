# SkyWatch - Custom ESP32 Quadcopter Drone

![SkyWatch Drone](images/drone-on-laptop.jpg)

**SkyWatch** is a fully open-source, WiFi-controlled mini quadcopter built on ESP32.  
It uses real-time PID control, IMU attitude estimation, live telemetry, and simple serial commands. Perfect for learning drone flight control, IMU calibration, and ESP32 programming.

---

## Features
- Full PID stabilization (Roll, Pitch, Yaw)
- IMU (accelerometer + gyro) with calibration & validity check
- WiFi client mode + WebSocket telemetry
- Serial commands for calibration, motor test, logging, reset
- Live serial monitor output
- Low-pass filtered derivative + anti-windup PID
- Ready for FPV/Whoop-style flying

---

## Hardware
- ESP32 DevKit / NodeMCU
- 4x coreless motors + ESCs (or direct PWM)
- IMU (MPU6050 or similar – see `imu.ino`)
- 1S/2S LiPo battery (shown on scale)
- Custom 3D-printed or tape frame
- LEDs for status (blue = armed, red = error)

**Components shown in photos:**
![Components on digital scale](images/components-on-scale.jpg)
![Full drone on laptop](images/drone-on-laptop2.jpg)
![Close-up with blue LED](images/drone-closeup-blue.jpg)

---

## Gallery (Flix-Style Drone Shots)
I added **flix-style** (cinematic, clean, movie-like) drone photos for your README header and gallery.  
Upload the ones you sent me and use these exact markdown lines:

```markdown
<image-card alt="Flix-style SkyWatch Drone 1" src="images/drone-flix1.jpg" ></image-card>
<image-card alt="Flix-style SkyWatch Drone 2" src="images/drone-flix2.jpg" ></image-card>
<image-card alt="Workbench build" src="images/workbench-magnifier.jpg" ></image-card>
<image-card alt="Drone with red LED" src="images/drone-red-led.jpg" ></image-card>
<image-card alt="Full setup with multimeter" src="images/setup-multimeter.jpg" ></image-card>