// flix.ino
// Main firmware file for FLiX + Wi-Fi web log / power debug

#include "vector.h"
#include "quaternion.h"
#include "util.h"

#include <esp_system.h>
#include <WiFi.h>
#include <WebServer.h>
// flip code 
#include "hw_esp32.h"
#include "attitude.h"
#include "controller.h"
#include "modes.h"
#include "mixer.h"
#include "config.h"

#define WIFI_ENABLED 1

// --------- Flight state (original FLiX globals) ----------
float t = NAN; // current step time, s
float dt;      // time delta from previous step, s

float controlRoll, controlPitch, controlYaw, controlThrottle; // pilot inputs [-1, 1]
float controlMode = NAN;

Vector gyro;      // gyroscope data
Vector acc;       // accelerometer data, m/s/s
Vector rates;     // filtered angular rates, rad/s
Quaternion attitude; // estimated attitude
bool landed;      // are we landed and stationary
float motors[4];  // normalized motor thrust [0..1]

// ------------- POWER DEBUG (RTC, survives soft reset) -------------
RTC_DATA_ATTR int   bootCount = 0;
RTC_DATA_ATTR float lastThrottleBeforeReset = 0.0f;

// ------------- WEB LOG / SERIAL MONITOR OVER WIFI -------------
WebServer webServer(80);
String webLogBuffer;                 // ring buffer of recent logs for WEB view
const size_t WEB_LOG_MAX_LEN = 4000; // ~4 KB of logs

// --------- Forward declarations from other modules ----------
void setupParameters();
void setupLED();
void setupMotors();
void setupIMU();
void setupRC();
void setupWiFi();   // defined in wifi.ino
void setLED(bool on);
void readIMU();
void step();
void readRC();
void estimate();
void control();
void sendMotors();
void handleInput();
void processMavlink();
void logData();
void syncParameters();
// optional in your tree:
// void disableBrownOut();

// --------- Helper: reset reason text ----------
const char* resetReasonToText(esp_reset_reason_t reason) {
  switch (reason) {
    case ESP_RST_POWERON:   return "POWERON";
    case ESP_RST_EXT:       return "EXT";
    case ESP_RST_SW:        return "SW";
    case ESP_RST_PANIC:     return "PANIC";
    case ESP_RST_INT_WDT:   return "INT_WDT";
    case ESP_RST_TASK_WDT:  return "TASK_WDT";
    case ESP_RST_WDT:       return "WDT";
    case ESP_RST_DEEPSLEEP: return "DEEPSLEEP";
    case ESP_RST_BROWNOUT:  return "BROWNOUT";
    case ESP_RST_SDIO:      return "SDIO";
    default:                return "UNKNOWN";
  }
}

// --------- Helper: add a line to web log buffer + Serial ----------
void logLine(const String &line) {
  // USB serial (safe even if no cable)
  Serial.println(line);

  // Append to web buffer
  webLogBuffer += line + "\n";
  if (webLogBuffer.length() > WEB_LOG_MAX_LEN) {
    // Drop oldest part (keep last WEB_LOG_MAX_LEN chars)
    int cutPos = webLogBuffer.length() - WEB_LOG_MAX_LEN;
    int nl = webLogBuffer.indexOf('\n', cutPos);
    if (nl >= 0) {
      webLogBuffer = webLogBuffer.substring(nl + 1);
    } else {
      webLogBuffer = webLogBuffer.substring(cutPos);
    }
  }
}

// --------- Web page served by ESP32 ----------
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <title>FLiX Drone Log</title>
  <style>
    body { font-family: monospace; background:#111; color:#0f0; }
    #log { white-space: pre-wrap; border:1px solid #444; padding:8px;
           max-height:90vh; overflow-y:scroll; }
  </style>
</head>
<body>
  <h2>FLiX Drone Log (Wi-Fi)</h2>
  <div id="status">Connecting...</div>
  <pre id="log"></pre>
  <script>
    async function fetchLog() {
      try {
        const res = await fetch('/log?ts=' + Date.now());
        const text = await res.text();
        const el = document.getElementById('log');
        el.textContent = text;
        el.scrollTop = el.scrollHeight;
        document.getElementById('status').textContent = 'Connected';
      } catch (e) {
        document.getElementById('status').textContent = 'Disconnected';
      }
      setTimeout(fetchLog, 500);
    }
    window.onload = fetchLog;
  </script>
</body>
</html>
)rawliteral";

void handleRoot() {
  webServer.send_P(200, "text/html", INDEX_HTML);
}

void handleLog() {
  webServer.send(200, "text/plain", webLogBuffer);
}

// --------- SETUP ----------
void setup() {
  Serial.begin(115200);
  print("Initializing flix\n");
  // flipe 
  hw_init();
  modes_init();

  bootCount++;
  esp_reset_reason_t reason = esp_reset_reason();

  // Core initialization
  setupParameters();
  setupLED();
  setupMotors();
  setLED(true);

#if WIFI_ENABLED
  setupWiFi();   // starts AP + MAVLink UDP (wifi.ino)
#endif

  // Start web server for log view
  webServer.on("/", handleRoot);
  webServer.on("/log", handleLog);
  webServer.begin();

  // Log power/debug header (visible via web + Serial)
  IPAddress ip = WiFi.softAPIP();  // AP IP when in AP mode
  logLine("========== FLIX POWER DEBUG ==========");
  logLine("Boot count: " + String(bootCount));
  logLine("Last throttle before reset: " + String(lastThrottleBeforeReset, 3));
  logLine(String("Reset reason: ") + resetReasonToText(reason));
  logLine("AP IP: " + ip.toString());
  logLine("======================================");

  // For debugging brownouts, you usually want brownout detector ON.
  // If you have a disableBrownOut() helper and want to disable it for flight:
  // disableBrownOut();

  setupIMU();
  setupRC();
  setLED(false);
  print("Initializing complete\n");
}

// --------- LOOP ----------
void loop() {
  // Handle HTTP clients for web log
  webServer.handleClient();

  // Original FLiX control pipeline
  readIMU();
  step();
  readRC();
  estimate();
  control();
  // flipe code
  static uint32_t last = micros();
    uint32_t now = micros();
    float dt = (now - last) * 1e-6f;
    if (dt < LOOP_DT) return;
    last = now;

    // 1. Read hardware
    float gx, gy, gz, ax, ay, az;
    float vbat;
    float thr, rc_roll, rc_pitch, rc_yaw;
    bool flip_button;

    hw_read_imu(gx, gy, gz, ax, ay, az);
    hw_read_battery(vbat);
    hw_read_rc(thr, rc_roll, rc_pitch, rc_yaw, flip_button);

    // 2. Estimate attitude
    Attitude att = attitude_update(gx, gy, gz, ax, ay, az, dt);

    // 3. Update modes & safety
    modes_update(att, thr, vbat, flip_button, dt);

    // 4. Control
    float m1 = 0, m2 = 0, m3 = 0, m4 = 0;
    if (is_armed()) {
        controller_update(att, thr,
                          rc_roll, rc_pitch, rc_yaw,
                          m1, m2, m3, m4, dt);
    }

    // 5. Write motors
    hw_write_motors(m1, m2, m3, m4);

  // Before sending motors, remember max requested thrust for debugging
  float maxMotor = motors[0];
  if (motors[1] > maxMotor) maxMotor = motors[1];
  if (motors[2] > maxMotor) maxMotor = motors[2];
  if (motors[3] > maxMotor) maxMotor = motors[3];

  lastThrottleBeforeReset = maxMotor;

  // Log every 0.5 s (to see what was happening just before power loss)
  static unsigned long lastLogMs = 0;
  unsigned long now = millis();
  if (now - lastLogMs > 500) {
    lastLogMs = now;
    logLine("t=" + String(now / 1000.0f, 2) +
            " s  maxMotor=" + String(maxMotor, 3));
  }

  sendMotors();
  handleInput();
#if WIFI_ENABLED
  processMavlink();
#endif
  logData();
  syncParameters();
}
