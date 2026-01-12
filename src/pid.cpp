#include "pid.h"   // flipe code

float pid_step(PID &pid, float error, float dt)
{
    pid.i_term += error * pid.ki * dt;

    float d = (error - pid.last_error) / dt;
    pid.last_error = error;

    return pid.kp * error + pid.i_term + pid.kd * d;
}
