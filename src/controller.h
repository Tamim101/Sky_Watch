#pragma once
#include "attitude.h"     // flipe code

void controller_update(const Attitude &att,
                       float thr,
                       float rc_roll, float rc_pitch, float rc_yaw,
                       float &m1, float &m2, float &m3, float &m4,
                       float dt);


#!/usr/bin/env python3
"""
FLiX FPV CONTROL (DragonRise) — FIXED (Gamepad-friendly throttle hold + KILL switch + safe axis)

Key fixes vs your version:
1) Throttle HOLD (integrator): throttle stays where you set it even when stick returns to center.
2) Emergency KILL button: instant throttle=0 + disarm.
3) Safe axis/center indexing: no crashes if your joystick reports fewer axes.
4) Keeps: IDLE throttle floor, post-arm zero delay, takeoff neutralize based on pilot throttle target.
"""

import time
import pygame
from flix import Flix

# ===================== MANUAL MAPPING =====================
# DragonRise common mapping (adjust if needed)
LX_AXIS = 0   # Left stick X (Yaw)
LY_AXIS = 1   # Left stick Y (Throttle)
RX_AXIS = 3   # Right stick X (Roll)
RY_AXIS = 4   # Right stick Y (Pitch) - Reverted to 4, as some DragonRise variants use this for right Y, and 5 may not work

LX_SIGN = +1.0
LY_SIGN = -1.0  # IMPORTANT: stick UP should become positive
RX_SIGN = +1.0
RY_SIGN = -1.0
# ==========================================================

# ===================== SAFETY / CONTROL ===================
# Pick a gamepad button for emergency kill (0 is common).
# Run a tiny pygame test if unsure, or just try 0/1/2 until it works.
KILL_BUTTON = 0

LOOP_HZ = 60
# ==========================================================

# ===================== FLIGHT TUNING ======================
# Deadzones
DZ_YAW   = 0.25
DZ_ROLL  = 0.12
DZ_PITCH = 0.12
DZ_THR   = 0.08   # throttle stick deadzone for the integrator

# Expo
EXPO_YAW   = 0.25
EXPO_ROLL  = 0.35
EXPO_PITCH = 0.35

# Max command limits
YAW_MAX = 0.55
RP_MAX  = 0.55

# Throttle limits
THR_CAP   = 0.70
IDLE_THR  = 0.10     # idle floor when armed (after post-arm delay)

# Throttle integrator rate (how fast stick changes throttle target)
THR_RATE_PER_TICK = 0.012  # tune 0.008..0.020

# Throttle ramp/smoothing (toward desired)
THR_RAMP_UP_PER_TICK   = 0.015
THR_RAMP_DOWN_PER_TICK = 0.030

# Arming gesture
ARM_HOLD_TIME = 0.5
RIGHT_CENTER_TOL = 0.22
THR_ARM_MAX = 0.05
POST_ARM_ZERO_SEC = 0.25

# Takeoff stability: neutralize attitude when pilot throttle target is low
NEUTRALIZE_BELOW_THR = 0.06
# ==========================================================


def clamp(x, lo, hi):
    return lo if x < lo else hi if x > hi else x


def dz(x, dead):
    return 0.0 if abs(x) < dead else x


def expo(x, e):
    return (1.0 - e) * x + e * (x ** 3)


def connect_flix():
    print("Connecting to Flix drone...")
    f = Flix(system_id=1, wait_connection=True)
    print(f"Connected! Mode: {f.mode}, Armed: {f.armed}")
    return f


def init_gamepad():
    pygame.init()
    pygame.joystick.init()
    if pygame.joystick.get_count() == 0:
        raise RuntimeError("No gamepad found! Plug it in.")
    js = pygame.joystick.Joystick(0)
    js.init()
    print(f"Gamepad: {js.get_name()} | Axes: {js.get_numaxes()} | Buttons: {js.get_numbuttons()} | Hats: {js.get_numhats()}")
    return js


def measure_center(js, seconds=1.0):
    print(f"Calibrating centers... keep sticks untouched for {seconds:.1f}s")
    t_end = time.time() + seconds
    n_axes = js.get_numaxes()
    sums = [0.0] * n_axes
    n = 0
    while time.time() < t_end:
        pygame.event.pump()
        for i in range(n_axes):
            sums[i] += js.get_axis(i)
        n += 1
        time.sleep(0.01)
    center = [s / n for s in sums] if n > 0 else [0.0] * n_axes
    print("Center:", [round(x, 4) for x in center])
    return center


def safe_center(center, i):
    return center[i] if 0 <= i < len(center) else 0.0


def axis(js, i):
    if i < 0 or i >= js.get_numaxes():
        return 0.0
    return js.get_axis(i)


def button(js, i):
    if i < 0 or i >= js.get_numbuttons():
        return 0
    return js.get_button(i)


def read_controls(js, center):
    pygame.event.pump()

    # Physical axes (center-compensated)
    phys_lx = (axis(js, LX_AXIS) - safe_center(center, LX_AXIS)) * LX_SIGN  # yaw
    phys_ly = (axis(js, LY_AXIS) - safe_center(center, LY_AXIS)) * LY_SIGN  # throttle "delta"
    phys_rx = (axis(js, RX_AXIS) - safe_center(center, RX_AXIS)) * RX_SIGN  # roll
    phys_ry = (axis(js, RY_AXIS) - safe_center(center, RY_AXIS)) * RY_SIGN  # pitch

    # Deadzones + expo for attitude
    raw_lx = dz(phys_lx, DZ_YAW)
    raw_rx = dz(phys_rx, DZ_ROLL)
    raw_ry = dz(phys_ry, DZ_PITCH)

    yaw   = clamp(expo(clamp(raw_lx, -1.0, 1.0), EXPO_YAW),   -YAW_MAX, YAW_MAX)
    roll  = clamp(expo(clamp(raw_rx, -1.0, 1.0), EXPO_ROLL),  -RP_MAX,  RP_MAX)
    pitch = clamp(expo(clamp(raw_ry, -1.0, 1.0), EXPO_PITCH), -RP_MAX,  RP_MAX)

    # Throttle integrator delta:
    # stick up => increase throttle target, stick down => decrease
    thr_delta = 0.0
    if abs(phys_ly) >= DZ_THR:
        thr_delta = phys_ly * THR_RATE_PER_TICK

    kill = bool(button(js, KILL_BUTTON))

    return roll, pitch, yaw, thr_delta, kill, phys_lx, phys_ly, phys_rx, phys_ry


def main():
    flix = connect_flix()
    js = init_gamepad()
    center = measure_center(js, 1.0)

    print("\n" + "=" * 78)
    print("   FLiX FPV CONTROL READY (THROTTLE HOLD + IDLE 0.10 + TAKEOFF FIX + KILL)")
    print("=" * 78)
    print("ARM     → Left stick bottom-right (hold 0.5s)  [throttle target must be LOW]")
    print("DISARM  → Left stick bottom-left  (hold 0.5s)")
    print(f"KILL    → Button {KILL_BUTTON} (instant disarm)")
    print("MOVE    → Right stick: roll/pitch; Left stick: yaw; Throttle: push up/down to change, release to hold")
    print("NOTE    → roll/pitch/yaw neutralize when throttle target <", NEUTRALIZE_BELOW_THR)
    print("IDLE    → when armed, throttle ramps to", IDLE_THR)
    print("SAFETY  → REMOVE PROPS for first tests.")
    print("=" * 78 + "\n")

    period = 1.0 / LOOP_HZ
    last_print = 0.0

    arm_timer = None
    disarm_timer = None
    arm_lockout_until = 0.0

    thr_cmd = 0.0
    thr_target = 0.0  # IMPORTANT: persistent pilot throttle target (holds)
    armed_since = None

    try:
        while True:
            roll, pitch, yaw, thr_delta, kill, phys_lx, phys_ly, phys_rx, phys_ry = read_controls(js, center)
            now = time.time()

            # --------- EMERGENCY KILL (instant) ----------
            if kill:
                try:
                    flix.set_controls(0.0, 0.0, 0.0, 0.0)
                    flix.cli("disarm")
                except Exception:
                    pass
                thr_cmd = 0.0
                thr_target = 0.0
                armed_since = None
                print("\nKILL pressed → DISARMED")
                time.sleep(0.05)
                continue
            # --------------------------------------------

            # Update persistent throttle target (gamepad-friendly)
            thr_target = clamp(thr_target + thr_delta, 0.0, THR_CAP)

            # Gesture checks
            right_centered = (abs(phys_rx) < RIGHT_CENTER_TOL and abs(phys_ry) < RIGHT_CENTER_TOL)
            throttle_low = (thr_target <= THR_ARM_MAX)

            bottom = phys_ly <= -0.80
            right  = phys_lx >= +0.80
            left   = phys_lx <= -0.80

            # ARM gesture (bottom-right)
            if now >= arm_lockout_until and right_centered and throttle_low and bottom and right:
                if arm_timer is None:
                    arm_timer = now
                elif (now - arm_timer) >= ARM_HOLD_TIME and not flix.armed:
                    print("\nARMING DRONE!")
                    try:
                        flix.cli("stab")
                        flix.cli("arm")
                    except Exception as e:
                        print("\nARM command error:", e)
                    arm_lockout_until = now + 1.0
                    time.sleep(0.15)
            else:
                arm_timer = None

            # DISARM gesture (bottom-left)
            if right_centered and bottom and left:
                if disarm_timer is None:
                    disarm_timer = now
                elif (now - disarm_timer) >= ARM_HOLD_TIME and flix.armed:
                    print("\nDISARMING DRONE!")
                    try:
                        flix.cli("disarm")
                    except Exception as e:
                        print("\nDISARM command error:", e)
                    time.sleep(0.15)
            else:
                disarm_timer = None

            # Track arming transitions
            if flix.armed and armed_since is None:
                armed_since = now
                thr_cmd = 0.0
            if (not flix.armed) and armed_since is not None:
                armed_since = None
                thr_cmd = 0.0
                thr_target = 0.0  # reset on disarm

            # Throttle desired:
            if flix.armed:
                if armed_since is not None and (now - armed_since) < POST_ARM_ZERO_SEC:
                    thr_desired = 0.0
                else:
                    thr_desired = max(thr_target, IDLE_THR)

                # Ramp thr_cmd toward thr_desired
                if thr_desired > thr_cmd:
                    thr_cmd += THR_RAMP_UP_PER_TICK
                else:
                    thr_cmd -= THR_RAMP_DOWN_PER_TICK

                thr_cmd = clamp(thr_cmd, 0.0, THR_CAP)
            else:
                thr_cmd = 0.0

            # ========= TAKEOFF ROLL FIX =========
            # Neutralize attitude when pilot throttle target is still low
            if thr_target < NEUTRALIZE_BELOW_THR:
                roll = 0.0
                pitch = 0.0
                yaw = 0.0
            # ===================================

            # Send controls
            try:
                flix.set_controls(roll=roll, pitch=pitch, yaw=yaw, throttle=thr_cmd)
            except Exception:
                # If disconnected, ignore; will resume when link returns
                pass

            # Status print
            if now - last_print > 0.10:
                status = "ARMED" if flix.armed else "DISARM"
                print(
                    f"\r{status} ThrCmd:{thr_cmd:.2f} ThrTgt:{thr_target:.2f} "
                    f"Y:{yaw:+.2f} R:{roll:+.2f} P:{pitch:+.2f} "
                    f"| physLX:{phys_lx:+.2f} physLY:{phys_ly:+.2f}   ",
                    end="", flush=True
                )
                last_print = now

            time.sleep(period)

    except KeyboardInterrupt:
        print("\n\nShutting down...")
    finally:
        print("\nSending zero throttle and quitting…")
        try:
            flix.set_controls(0.0, 0.0, 0.0, 0.0)
            if flix.armed:
                flix.cli("disarm")
        except Exception:
            pass
        pygame.quit()
        print("Safely stopped.")


if __name__ == "__main__":
    main()