# Sky_Watch
> make log
Invalid command: make
> log
valid,t,rates.x,rates.y,rates.z,ratesTarget.x,ratesTarget.y,ratesTarget.z,attitude.x,attitude.y,attitude.z,attitudeTarget.x,attitudeTarget.y,attitudeTarget.z,thrustTarget
> imu
status: OK
model: MPU-6500 / clone
who am I: 0x70
rate: 520
gyro: 0.099117 0.061930 -0.092612
acc:  -1.287238 -4.756902 8.143799
acc_mag: 9.519
landed: 1
gyro bias:  -0.008719 0.000920 0.008538
accel bias: 0.000000 0.000000 0.000000
accel scale:1.099232 1.099232 1.099232
gyro bias:  -0.008719 0.000920 0.008538
accel bias: 0.000000 0.000000 0.000000
accel scale:1.099232 1.099232 1.099232
landed: 1
> rc
channels: 1500 1500 1000 1500 1600 0 0 0 0 0 0 0 0 0 0 0 
roll: 0 pitch: 0 yaw: 0 throttle: 0 mode: 0.6
mode: STAB
armed: 0
> rc
channels: 1500 1500 1000 1500 1600 0 0 0 0 0 0 0 0 0 0 0 
roll: 0 pitch: 0 yaw: 0 throttle: 0 mode: 0.6
mode: STAB
armed: 1









> p
CTL_R_RATE_P = 0.05
CTL_R_RATE_I = 0.2
CTL_R_RATE_D = 0.001
CTL_R_RATE_WU = 0.3
CTL_P_RATE_P = 0.05
CTL_P_RATE_I = 0.2
CTL_P_RATE_D = 0.001
CTL_P_RATE_WU = 0.3
CTL_Y_RATE_P = 0.3
CTL_Y_RATE_I = 0
CTL_Y_RATE_D = 0
CTL_Y_RATE_WU = 0.3
CTL_R_P = 6
CTL_R_I = 0
CTL_R_D = 0
CTL_P_P = 6
CTL_P_I = 0
CTL_P_D = 0
CTL_Y_P = 3
CTL_P_RATE_MAX = 4
CTL_R_RATE_MAX = 4
CTL_Y_RATE_MAX = 5.23599
CTL_TILT_MAX = 0.35
IMU_G_BIAS_X = -0.00871904
IMU_G_BIAS_Y = 0.000919797
IMU_G_BIAS_Z = 0.00853813
IMU_ACC_BIAS_X = 0
IMU_ACC_BIAS_Y = 0
IMU_ACC_BIAS_Z = 0
IMU_ACC_SCL_X = 1.09923
IMU_ACC_SCL_Y = 1.09923
IMU_ACC_SCL_Z = 1.09923
EST_ACC_W = 0.003
EST_R_LPF_A = 0.2
RC_ZERO_0 = 1000
RC_ZERO_1 = 1000
RC_ZERO_2 = 1000
RC_ZERO_3 = 1000
RC_ZERO_4 = 1000
RC_ZERO_5 = 1000
RC_ZERO_6 = 1000
RC_ZERO_7 = 1000
RC_CEN_0 = 1500
RC_CEN_1 = 1500
RC_CEN_2 = 1500
RC_CEN_3 = 1500
RC_CEN_4 = 1500
RC_CEN_5 = 1500
RC_CEN_6 = 1500
RC_CEN_7 = 1500
RC_MAX_0 = 2000
RC_MAX_1 = 2000
RC_MAX_2 = 2000
RC_MAX_3 = 2000
RC_MAX_4 = 2000
RC_MAX_5 = 2000
RC_MAX_6 = 2000
RC_MAX_7 = 2000
> help

Welcome to
 _______  __       __  ___   ___
|   ____||  |     |  | \  \ /  /
|  |__   |  |     |  |  \  V  /
|   __|  |  |     |  |   >   <
|  |     |  `----.|  |  /  .  \
|__|     |_______||__| /__/ \__\

Commands:

help - show help
p - show all parameters
p <name> - show parameter
p <name> <value> - set parameter
preset - reset parameters
time - show time info
ps - show pitch/roll/yaw
psq - show attitude quaternion
imu - show IMU data
arm - arm the drone
disarm - disarm the drone
raw/stab/acro/auto - set mode
rc - show RC data
wifi - show Wi-Fi info
mot - show motor output
log [dump] - print log header [and data]
cr - calibrate RC
ca - calibrate accel
mfr, mfl, mrr, mrl - test motor (remove props)
sys - show system info
reset - reset drone's state
reboot - reboot the drone

> imu
status: OK
model: MPU-6500 / clone
who am I: 0x70
rate: 484
gyro: 0.011851 -0.061520 0.034031
acc:  4.957284 -4.194960 7.170201
acc_mag: 9.674
landed: 1
gyro bias:  -0.008719 0.000920 0.008538
accel bias: 0.000000 0.000000 0.000000
accel scale:1.099232 1.099232 1.099232
gyro bias:  -0.008719 0.000920 0.008538
accel bias: 0.000000 0.000000 0.000000
accel scale:1.099232 1.099232 1.099232
landed: 1
> mrr
Testing motor 1
Done
> mfl
Testing motor 3
Done
> mfr
Testing motor 2
Done
> mrl
Testing motor 0
Done
> cr
RC calibration not used in Wi-Fi / MAVLink mode
Control   Ch     Zero   Center Max
Roll     0      1000   1500   2000   
Pitch    1      1000   1500   2000   
Yaw      3      1000   1500   2000   
Throttle 2      1000   1500   2000   
Mode     4      1000   1500   2000   
> ca
Calibrating accel (6 faces)...
Do NOT touch during sampling.
1/6: place on a new face and hold still [8 s]
2/6: place on a new face and hold still [8 s]
3/6: place on a new face and hold still [8 s]
4/6: place on a new face and hold still [8 s]
5/6: place on a new face and hold still [8 s]
> ps
roll: -85.971181 pitch: 50.762881 yaw: 16.129719
6/6: place on a new face and hold still [8 s]
Accel calibration done.
gyro bias:  -0.008719 0.000920 0.008538
accel bias: -5.934193 -7.576795 0.393183
accel scale:0.023590 0.013997 0.092937
> sys
Chip: ESP32-C3
Temperature: 41.2 °C
Free heap: 149628




> imu
status: OK
model: MPU-6500 / clone
who am I: 0x70
rate: 542
gyro: -0.001393 -0.000991 0.000318
acc:  -0.038892 -0.002161 9.876383
acc_mag: 9.876
landed: 1
gyro bias:  -0.035193 0.002457 0.027352
accel bias: 0.000000 0.000000 0.000000
accel scale:1.108088 1.108088 1.108088
gyro bias:  -0.035193 0.002457 0.027352
accel bias: 0.000000 0.000000 0.000000
accel scale:1.108088 1.108088 1.108088
landed: 1
