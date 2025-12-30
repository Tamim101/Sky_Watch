// rc.ino – RC interface + stub for Wi-Fi/MAVLink-only setup
// Works on ESP32-C3 without physical SBUS receiver.

#include "util.h"

// Raw channels
uint16_t channels[16] = {0};

// Time of last controls update
float controlTime = 0.0f;

// Calibration
float channelZero[16] = {0};
float channelMax[16]  = {
    1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1
};

// Channel mapping
float rollChannel     = NAN;
float pitchChannel    = NAN;
float throttleChannel = NAN;
float yawChannel      = NAN;
float modeChannel     = NAN;

// Provided elsewhere
extern float controlRoll;
extern float controlPitch;
extern float controlYaw;
extern float controlThrottle;
extern float controlMode;
extern float t;  // current time (from main)

// --------------------------------------------------
// With real SBUS receiver
// --------------------------------------------------
#if RC_ENABLED

#include <SBUS.h>
SBUS rc(Serial1);

void setupRC() {
    print("Setup RC (SBUS)\n");
    rc.begin();
}

void normalizeRC() {
    float controls[16];
    for (int i = 0; i < 16; i++) {
        controls[i] = mapf(channels[i], channelZero[i], channelMax[i], 0, 1);
    }
    controlRoll     = rollChannel     >= 0 ? controls[(int)rollChannel]     : NAN;
    controlPitch    = pitchChannel    >= 0 ? controls[(int)pitchChannel]    : NAN;
    controlYaw      = yawChannel      >= 0 ? controls[(int)yawChannel]      : NAN;
    controlThrottle = throttleChannel >= 0 ? controls[(int)throttleChannel] : NAN;
    controlMode     = modeChannel     >= 0 ? controls[(int)modeChannel]     : NAN;
}

void readRC() {
    // Called every loop() from flix.ino – no return value
    if (rc.read()) {
        SBUSData data = rc.data();
        for (int i = 0; i < 16; i++) {
            channels[i] = data.ch[i];
        }
        normalizeRC();
        controlTime = t;
    }
}

// keep your original calibrateRC(), calibrateRCChannel(), printRCCalibration()
// in this RC_ENABLED branch if you had them before

#else  // !RC_ENABLED → Wi-Fi / MAVLink only

void setupRC() {
    print("Setup RC (disabled, Wi-Fi / MAVLink control only)\n");
    for (int i = 0; i < 16; i++) {
        channels[i]    = 0;
        channelZero[i] = 1000;
        channelMax[i]  = 2000;
    }
    rollChannel     = -1;
    pitchChannel    = -1;
    throttleChannel = -1;
    yawChannel      = -1;
    modeChannel     = NAN;
    controlTime     = 0;
}

void readRC() {
    // Nothing – controls come from MAVLink/manual_control
}

void normalizeRC() {}

void calibrateRC() {
    print("RC calibration skipped (RC_DISABLED)\n");
}

void printRCCalibration() {
    print("RC disabled, using MAVLink controls only\n");
}

#endif
