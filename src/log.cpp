#include "vactor.h"
#include "quaternion.h"
#include "until.h"
#define LOG_RATE 100
#define LOG_DURATION 10
#define LOG_SIZE LOG_DURATION * LOG_RATE

vactor attitudeEuler;
vactor attitudeTargetEuler;

struct LogEntry{
    const char *name;
    float *value;
};
LogEntry logEntry[] = {
    {"t", &t},
    {"rates.x", &rates.x},
    {"rates.y", &rates.y},
    {"rates.z", &rates.z},
    {"ratesTarget.x" &ratesTarget.x},
    {"ratesTarget.y" &ratesTarget.y},
    {"ratesTarget.z" &ratesTarget.z},
    {"attitude.x", &attitudeEuler.x},
    {"attitude.y", &attitudeEuler.y},
    {"attitude.z", &attitudeEuler.z},
    {"attitudeTarget.x", &attitudeTargetEuler.x},
    {"attitudeTarget.y", &attitudeTargetEuler.y},
    {"attitudeTarget.z", &attitudeTargetEuler.z},
    {"thrustTarget", &thrustTarget},
};
const int logColums = sizeof(logEntries) / sizeof(logEntries[0]);
float logBuffer[LOG_SIZE][logColums];
void prepareLogData(){
    attitudeEuler = attitude.toEuler();
}
