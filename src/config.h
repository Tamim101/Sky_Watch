#pragma once   // flipe code

#define LOOP_HZ 500
#define LOOP_DT (1.0f / LOOP_HZ)

// Angle limits
#define ANGLE_LIMIT 70.0f

// Battery cutoff
#define VBAT_WARN 3.2f
#define VBAT_CUTOFF 3.0f

// Flip parameters
#define FLIP_RATE 800.0f     // deg/sec
#define FLIP_ACCEL_TIME 0.18f
#define FLIP_BRAKE_TIME 0.06f