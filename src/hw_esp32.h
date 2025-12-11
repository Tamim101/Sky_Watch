#pragma once   // flipe code

void hw_init();
void hw_read_imu(float &gx, float &gy, float &gz,
                 float &ax, float &ay, float &az);
void hw_read_battery(float &vbat);
void hw_read_rc(float &thr, float &roll, float &pitch,
                float &yaw, bool &flip_button);
void hw_write_motors(float m1, float m2, float m3, float m4);
