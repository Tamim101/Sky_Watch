// Motors output control using MOSFETs – ESP32-C3 version

#include "util.h"

#define MOTOR_0_PIN 10  // rear left
#define MOTOR_1_PIN 4   // rear right
#define MOTOR_2_PIN 5   // front right
#define MOTOR_3_PIN 6   // front left

// Good combo for ESP32-C3 + brushed motors
#define PWM_FREQUENCY   16000           // 16 kHz (quiet, safe)
#define PWM_RESOLUTION  8               // 8-bit (0..255)

// PWM in microseconds equivalent (period = 1e6 / freq)
#define PWM_STOP        0
#define PWM_MIN         0
#define PWM_MAX         (1000000.0f / PWM_FREQUENCY)

// Motors array indexes:
const int MOTOR_REAR_LEFT   = 0;
const int MOTOR_REAR_RIGHT  = 1;
const int MOTOR_FRONT_RIGHT = 2;
const int MOTOR_FRONT_LEFT  = 3;

void setupMotors() {
    print("Setup Motors\n");

    // Flix’s util.h has ledcAttach(pin, freq, res) wrapper
    ledcAttach(MOTOR_0_PIN, PWM_FREQUENCY, PWM_RESOLUTION);
    ledcAttach(MOTOR_1_PIN, PWM_FREQUENCY, PWM_RESOLUTION);
    ledcAttach(MOTOR_2_PIN, PWM_FREQUENCY, PWM_RESOLUTION);
    ledcAttach(MOTOR_3_PIN, PWM_FREQUENCY, PWM_RESOLUTION);

    sendMotors();  // start with all zeros
    print("Motors initialized\n");
}

int getDutyCycle(float value) {
    value = constrain(value, 0.0f, 1.0f);

    // Map [0..1] → [PWM_MIN..PWM_MAX] in microseconds
    float pwm = mapf(value, 0.0f, 1.0f, PWM_MIN, PWM_MAX);
    if (value == 0.0f) pwm = PWM_STOP;

    // Map microseconds → duty counts [0..(2^RES-1)]
    float duty = mapf(pwm,
                      0.0f,
                      PWM_MAX,
                      0.0f,
                      (1 << PWM_RESOLUTION) - 1);

    return (int)roundf(duty);
}

void sendMotors() {
    ledcWrite(MOTOR_0_PIN, getDutyCycle(motors[0]));
    ledcWrite(MOTOR_1_PIN, getDutyCycle(motors[1]));
    ledcWrite(MOTOR_2_PIN, getDutyCycle(motors[2]));
    ledcWrite(MOTOR_3_PIN, getDutyCycle(motors[3]));
}

bool motorsActive() {
    return motors[0] != 0 || motors[1] != 0 || motors[2] != 0 || motors[3] != 0;
}

void testMotor(int n) {
    print("Testing motor %d\n", n);
    motors[n] = 1.0f;   // full power for that motor
    delay(50);          // let LEDC update
    sendMotors();
    pause(3);           // run 3 seconds
    motors[n] = 0.0f;
    sendMotors();
    print("Done\n");
}
