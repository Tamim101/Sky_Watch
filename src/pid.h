#pragma once             // flipe code

struct PID {
    float kp, ki, kd;
    float i_term;
    float last_error;
};

float pid_step(PID &pid, float error, float dt);
