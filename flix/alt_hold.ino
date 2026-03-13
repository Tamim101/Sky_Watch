// alt_hold.ino - basic altitude hold in AUTO mode
#include "pid.h"
#include "util.h"
#include "vector.h"
#include "quaternion.h"

extern int mode;
extern bool armed;
extern Quaternion attitudeTarget;
extern Quaternion attitude;
extern Vector ratesExtra;
extern float thrustTarget;

extern const int AUTO;

extern float alt;     // from baro.ino
extern float altVel;  // from baro.ino

// --- Tunables (also exposed as parameters below) ---
PID altPID(1.5f, 0.0f, 0.4f, 0.2f);   // P, I, D, windup
float altHoverThrust = 0.70f;         // hover thrust guess (tune!)
float altMax = 1.5f;                  // safety clamp (meters)

// --- State ---
bool altHoldEnabled = false;
float altRef = 0.0f;       // altitude when enabling hold
float altTargetRel = 0.0f; // meters above altRef

void setAltHold(bool en) {
  altHoldEnabled = en;
  altPID.reset();
  if (en) {
    altRef = alt;          // current altitude becomes “0”
  }
}

void setAltTarget(float meters) {
  altTargetRel = constrain(meters, 0.0f, altMax);
  if (!altHoldEnabled) setAltHold(true);
}

void updateAltHold() {
  if (mode != AUTO) return;
  if (!altHoldEnabled) return;

  float target = altRef + altTargetRel;
  float error = target - alt;

  // PID output is "thrust delta"
  float u = altPID.update(error);

  // Small extra damping using measured vertical speed (optional)
  u += (-0.10f * altVel);

  float cmd = altHoverThrust + u;
  thrustTarget = constrain(cmd, 0.0f, 1.0f);

  // keep level attitude, keep yaw
  float yaw = attitude.getYaw();
  if (attitudeTarget.valid()) yaw = attitudeTarget.getYaw();
  attitudeTarget = Quaternion::fromEuler(Vector(0, 0, yaw));
  ratesExtra = Vector(0, 0, 0);

  // arm logic: thrust > 0 => armed
  if (thrustTarget > 0.05f) armed = true;
  if (altTargetRel < 0.02f && thrustTarget < 0.05f) armed = false;
}