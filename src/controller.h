#pragma once
#include "attitude.h"     // flipe code

void controller_update(const Attitude &att,
                       float thr,
                       float rc_roll, float rc_pitch, float rc_yaw,
                       float &m1, float &m2, float &m3, float &m4,
                       float dt);


