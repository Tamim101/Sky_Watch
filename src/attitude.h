#pragma once             // flipe code 

struct Attitude {
    float roll;
    float pitch;
    float yaw_rate;
};

Attitude attitude_update(float gx, float gy, float gz,
                         float ax, float ay, float az,
                         float dt);
