/*
  SeedStick main firmware (firmware/README.md)
  ==========================================
  Planting cycle: push the stick in, read the moisture band (OLED + green LED), tap the touch
  button, the servo turns the metering wheel by 180 degrees, the IR sensor confirms the falling ball, count and log.

  Touch button:  tap (< 0.8 s) = set one ball,  hold 2 s = Wi-Fi on/off,  hold 5 s = moisture calibration.
  Serial monitor (115200 baud, line ending "New Line"): a start report after every reset; "test" checks every
  part one after the other, "help" lists the commands (servo end positions, status, log, rotate). Pins and start
  values: config.h. Upload and first start: firmware/README.md.
  Web console (1.2): with Wi-Fi on, http://192.168.4.1/console shows the serial output and takes the same
  commands, so no USB cable is needed for adjustments. Only with WIFI_PASSWORD in secrets.h.

  Board: "ESP32 Dev Module", esp32 core 3.3.12. Library: U8g2 2.36.19 (Library Manager, "U8g2 by oliver").
  POWER: main switch off before plugging in USB. To work with USB and servo power at the same time, first pull the
  yellow 5V wire out of hole 2b; afterwards: switch off, USB out, yellow wire back into 2b.
*/
#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <LittleFS.h>
#include <U8g2lib.h>
#include <esp_system.h>
#include <sys/time.h>
#include <time.h>

#include "config.h"
#if __has_include("secrets.h")
#include "secrets.h"
#endif
#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD nullptr
#endif

static const char* FW_VERSION = "1.2";
static const char* WIFI_PASS = WIFI_PASSWORD;

U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled(U8G2_R0, U8X8_PIN_NONE);
Preferences prefs;
WebServer server(80);

enum class State : uint8_t { Ready, Dose, Confirmed, NoBall, Calibrate, Notice };
enum class Band : uint8_t { Dry, Good, Wet };
enum class RedMode : uint8_t { Off, Double, Fast };
enum class TouchEvent : uint8_t { None, Tap, HoldWifi, HoldCal };
enum class TestResult : uint8_t { Ok, Missing, Blocked, Look, Skipped };  // self-test ("test"); declared up here
                                                                         // so the IDE's prototypes know it

// ---------------------------------------------------------------------------------------------------- data
// calibration and counter, kept in flash (namespace "seedstick")
int servoUsA = SERVO_US_A_DEFAULT;
int servoUsB = SERVO_US_B_DEFAULT;
bool servoAtA = true;
int moistDryMv = MOIST_DRY_MV_DEFAULT;
int moistWetMv = MOIST_WET_MV_DEFAULT;
uint32_t ballCount = 0;

State state = State::Ready;
uint32_t stateSince = 0;

// servo
bool servoActive = false;
uint32_t servoOffAt = 0;      // 0 = keep the pulses on (serial adjustment)
uint32_t servoLastStop = 0;
char servoSelected = 'A';

// IR (interrupt)
volatile uint32_t irEdges = 0;
volatile uint32_t irLastEdgeMs = 0;
uint32_t irEdgesAtDose = 0;
uint32_t doseStartMs = 0;
bool doseBlockedAtStart = false;
bool irBlocked = false;
uint32_t irActiveSince = 0;
bool irWasActive = false;

// moisture and supply
int moistBuf[MOIST_AVG];
int moistIdx = 0;
int moistFill = 0;
int moistMv = 0;
int moistPct = 0;
Band band = Band::Dry;
uint32_t lastMoistMs = 0;
float supplyV = 0.0f;
uint32_t lastSupplyMs = 0;

// touch
bool touchRaw = false;
bool touchStable = false;
uint32_t touchRawSince = 0;
bool pressing = false;
uint32_t pressStart = 0;
uint32_t lockoutUntil = 0;

// LEDs, display, calibration, Wi-Fi, log
RedMode redMode = RedMode::Off;
uint32_t redSince = 0;
uint32_t lastDrawMs = 0;
char notice[3][24];
uint32_t noticeUntil = 0;
int calStep = 0;
int calDryMv = 0;
bool wifiOn = false;
bool timeSet = false;
int tzOffsetMin = 0;
uint32_t logNr = 0;
bool oledFound = false;             // the display answered on I2C at start
bool fsOk = false;                  // LittleFS mounted (log file)
bool oledRot180 = OLED_ROTATE_180;  // display turned by 180 degrees (command "rotate", kept in flash)

// ---------------------------------------------------------------------------------------------------- web console (1.2)
// Everything printed goes to Serial and into a ring buffer that the page /console fetches; lines typed on that page
// are read like serial input. The console only works when the access point has a password (WIFI_PASSWORD in
// secrets.h), so nobody nearby can move the servo or clear the log through an open network.
constexpr size_t CON_OUT_SIZE = 4096;
constexpr size_t CON_IN_SIZE = 256;
char conOut[CON_OUT_SIZE];
uint32_t conOutTotal = 0;  // characters written since the start (position for the page)
char conIn[CON_IN_SIZE];
size_t conInHead = 0, conInTail = 0;

bool consoleAllowed() { return WIFI_PASS != nullptr && strlen(WIFI_PASS) >= 8; }

class ConsoleOut : public Print {
 public:
  size_t write(uint8_t c) override {
    Serial.write(c);
    conOut[conOutTotal % CON_OUT_SIZE] = (char)c;
    conOutTotal++;
    return 1;
  }
  size_t write(const uint8_t* buf, size_t n) override {
    for (size_t i = 0; i < n; i++) write(buf[i]);
    return n;
  }
};
ConsoleOut Con;

void conPush(const char* s) {  // a line from the page into the input queue
  for (; *s; s++) {
    size_t next = (conInHead + 1) % CON_IN_SIZE;
    if (next == conInTail) return;  // queue full: rest dropped
    conIn[conInHead] = *s;
    conInHead = next;
  }
}

void serveWifi();  // with the web pages; keeps them alive while a command (self-test) waits for input

int conAvailable() {
  serveWifi();
  return Serial.available() + (int)((conInHead + CON_IN_SIZE - conInTail) % CON_IN_SIZE);
}

int conRead() {
  if (Serial.available()) return Serial.read();
  if (conInTail == conInHead) return -1;
  char c = conIn[conInTail];
  conInTail = (conInTail + 1) % CON_IN_SIZE;
  return (uint8_t)c;
}

void IRAM_ATTR onIrEdge() {
  uint32_t now = millis();
  if (now - irLastEdgeMs >= IR_DEBOUNCE_MS) {
    irLastEdgeMs = now;
    irEdges = irEdges + 1;  // not "irEdges++": deprecated on volatile in C++20
  }
}

// ---------------------------------------------------------------------------------------------------- helpers
void setState(State s) {
  state = s;
  stateSince = millis();
}

const char* bandName(Band b) { return b == Band::Dry ? "trocken" : (b == Band::Good ? "geeignet" : "nass"); }
const char* bandTitle(Band b) { return b == Band::Dry ? "TROCKEN" : (b == Band::Good ? "GEEIGNET" : "NASS"); }

void decimalComma(char* s) {
  for (; *s; s++) if (*s == '.') *s = ',';
}

void showNotice(const char* a, const char* b, const char* c, uint32_t ms) {
  strlcpy(notice[0], a, sizeof notice[0]);
  strlcpy(notice[1], b, sizeof notice[1]);
  strlcpy(notice[2], c, sizeof notice[2]);
  noticeUntil = millis() + ms;
  setState(State::Notice);
}

bool supplyMeasured() { return supplyV > 1.0f; }  // below 1 V: divider not fitted or no supply
bool supplyWarn() { return supplyMeasured() && supplyV < SUPPLY_WARN_V; }
bool supplyLow() { return supplyMeasured() && supplyV < SUPPLY_LOW_V; }

// ---------------------------------------------------------------------------------------------------- flash
void loadSettings() {
  prefs.begin("seedstick", false);
  servoUsA = prefs.getInt("servoA", SERVO_US_A_DEFAULT);
  servoUsB = prefs.getInt("servoB", SERVO_US_B_DEFAULT);
  servoAtA = prefs.getBool("atA", true);
  moistDryMv = prefs.getInt("moistDry", MOIST_DRY_MV_DEFAULT);
  moistWetMv = prefs.getInt("moistWet", MOIST_WET_MV_DEFAULT);
  ballCount = prefs.getUInt("count", 0);
  oledRot180 = prefs.getBool("rot180", OLED_ROTATE_180);
}

// ---------------------------------------------------------------------------------------------------- servo
void servoWriteUs(int us) {
  us = constrain(us, SERVO_US_MIN, SERVO_US_MAX);
  ledcWrite(PIN_SERVO, (uint32_t)us * (1UL << 14) / 20000UL);
}

void servoGo(int us, bool autoOff) {
  servoWriteUs(us);
  servoActive = true;
  servoOffAt = autoOff ? millis() + SERVO_MOVE_MS : 0;
}

void servoOff() {
  ledcWrite(PIN_SERVO, 0);
  if (servoActive) servoLastStop = millis();
  servoActive = false;
  servoOffAt = 0;
}

void servoUpdate() {
  if (servoActive && servoOffAt && (int32_t)(millis() - servoOffAt) >= 0) servoOff();
}

bool servoQuiet() { return !servoActive && millis() - servoLastStop >= MOIST_SERVO_QUIET_MS; }

// ---------------------------------------------------------------------------------------------------- sensors
int readMoistureOnce() {
  uint32_t sum = 0;
  for (int i = 0; i < MOIST_SAMPLES; i++) {
    sum += analogReadMilliVolts(PIN_MOIST);
    delayMicroseconds(100);
  }
  return sum / MOIST_SAMPLES;
}

int moisturePercent(int mv) {
  int span = moistDryMv - moistWetMv;
  if (span < 1) return 0;
  return constrain((moistDryMv - mv) * 100 / span, 0, 100);
}

Band bandOf(int pct) { return pct < BAND_DRY_PCT ? Band::Dry : (pct > BAND_WET_PCT ? Band::Wet : Band::Good); }

void updateMoisture() {
  if (millis() - lastMoistMs < MOIST_INTERVAL_MS || !servoQuiet()) return;
  lastMoistMs = millis();
  moistBuf[moistIdx] = readMoistureOnce();
  moistIdx = (moistIdx + 1) % MOIST_AVG;
  if (moistFill < MOIST_AVG) moistFill++;
  long sum = 0;
  for (int i = 0; i < moistFill; i++) sum += moistBuf[i];
  moistMv = sum / moistFill;
  moistPct = moisturePercent(moistMv);
  band = bandOf(moistPct);
}

float readSupplyNow() {
  uint32_t sum = 0;
  for (int i = 0; i < 16; i++) sum += analogReadMilliVolts(PIN_SUPPLY);
  return SUPPLY_DIVIDER * (sum / 16.0f) / 1000.0f;
}

void updateSupply() {
  if (millis() - lastSupplyMs < SUPPLY_INTERVAL_MS || !servoQuiet()) return;
  lastSupplyMs = millis();
  supplyV = readSupplyNow();
}

void updateIrBlocked() {
  bool active = digitalRead(PIN_IR) == IR_ACTIVE_LEVEL;
  if (active && !irWasActive) irActiveSince = millis();
  irWasActive = active;
  irBlocked = active && millis() - irActiveSince >= IR_BLOCKED_MS;
}

// ---------------------------------------------------------------------------------------------------- touch
TouchEvent readTouch() {
  uint32_t now = millis();
  if (now < TOUCH_SETTLE_MS) return TouchEvent::None;
  bool raw = digitalRead(PIN_TOUCH) == HIGH;
  if (raw != touchRaw) {
    touchRaw = raw;
    touchRawSince = now;
  }
  if (now - touchRawSince < TOUCH_DEBOUNCE_MS || touchRaw == touchStable) return TouchEvent::None;
  touchStable = touchRaw;
  if (touchStable) {
    pressing = true;
    pressStart = now;
    return TouchEvent::None;
  }
  pressing = false;
  uint32_t held = now - pressStart;
  if (held >= HOLD_CAL_MS) return TouchEvent::HoldCal;
  if (held >= HOLD_WIFI_MS) return TouchEvent::HoldWifi;
  if (held <= TAP_MAX_MS) return TouchEvent::Tap;
  return TouchEvent::None;
}

// ---------------------------------------------------------------------------------------------------- log
void formatClock(char* buf, size_t n) {
  if (!timeSet) {
    buf[0] = 0;
    return;
  }
  time_t t = time(nullptr) - (time_t)tzOffsetMin * 60;
  struct tm tmv;
  gmtime_r(&t, &tmv);
  strftime(buf, n, "%Y-%m-%d %H:%M:%S", &tmv);
}

void countLogLines() {
  logNr = 0;
  if (!fsOk || !LittleFS.exists(LOG_PATH)) return;
  File f = LittleFS.open(LOG_PATH, FILE_READ);
  if (!f) return;
  bool header = true;
  while (f.available()) {
    if (f.read() == '\n') {
      if (header) header = false;
      else logNr++;
    }
  }
  f.close();
}

void logEntry(const char* result) {
  if (!fsOk) return;
  bool exists = LittleFS.exists(LOG_PATH);
  File f = LittleFS.open(LOG_PATH, FILE_APPEND);
  if (!f) return;
  if (!exists) f.println("nr,zeit_s,uhrzeit,feuchte_pct,feuchte_mv,band,ergebnis,versorgung_v");
  logNr++;
  char clock[24];
  formatClock(clock, sizeof clock);
  f.printf("%lu,%lu,%s,%d,%d,%s,%s,%.2f\n", (unsigned long)logNr, (unsigned long)(millis() / 1000), clock, moistPct,
           moistMv, bandName(band), result, supplyV);
  f.close();
}

void clearLog() {
  if (fsOk) LittleFS.remove(LOG_PATH);
  logNr = 0;
  ballCount = 0;
  prefs.putUInt("count", 0);
}

// ---------------------------------------------------------------------------------------------------- dosing
void startDose() {
  doseBlockedAtStart = irBlocked;
  irEdgesAtDose = irEdges;
  doseStartMs = millis();
  servoAtA = !servoAtA;
  servoGo(servoAtA ? servoUsA : servoUsB, true);
  prefs.putBool("atA", servoAtA);
  lockoutUntil = doseStartMs + LOCKOUT_MS;
  setState(State::Dose);
}

void updateDose() {
  uint32_t edges = irEdges;
  uint32_t last = irLastEdgeMs;
  bool seen = edges != irEdgesAtDose && (int32_t)(last - doseStartMs) >= (int32_t)IR_IGNORE_MS;
  if (seen) {
    ballCount++;
    prefs.putUInt("count", ballCount);
    logEntry("ok");
    redMode = RedMode::Double;
    redSince = millis();
    setState(State::Confirmed);
    return;
  }
  if (millis() - doseStartMs < IR_WINDOW_MS) return;
  if (doseBlockedAtStart) {
    logEntry("ir_blockiert");
    showNotice("IR-Sensor", "blockiert:", "Fenster prüfen", 3000);
  } else {
    logEntry("keine_kugel");
    setState(State::NoBall);
  }
  redMode = RedMode::Fast;
  redSince = millis();
}

// ---------------------------------------------------------------------------------------------------- calibration
int measureMoistureNow() {
  long sum = 0;
  for (int i = 0; i < 8; i++) {
    sum += readMoistureOnce();
    delay(5);
  }
  return sum / 8;
}

void startCalibration() {
  calStep = 0;
  setState(State::Calibrate);
}

void calibrationTap() {
  if (calStep == 0) {
    calDryMv = measureMoistureNow();
    calStep = 1;
    return;
  }
  int wet = measureMoistureNow();
  if (calDryMv - wet >= MOIST_MIN_SPAN_MV) {
    moistDryMv = calDryMv;
    moistWetMv = wet;
    prefs.putInt("moistDry", moistDryMv);
    prefs.putInt("moistWet", moistWetMv);
    moistFill = 0;  // restart the moving average with the new scale
    char line[24];
    snprintf(line, sizeof line, "%d / %d mV", moistDryMv, moistWetMv);
    showNotice("Kalibriert", "trocken / nass:", line, 3000);
    Con.printf("Feuchte kalibriert: trocken %d mV, nass %d mV\n", moistDryMv, moistWetMv);
  } else {
    showNotice("Nicht gespeichert:", "trocken und nass", "zu nah beieinander", 4000);
    Con.printf("Kalibrierung verworfen: trocken %d mV, nass %d mV\n", calDryMv, wet);
  }
}

// ---------------------------------------------------------------------------------------------------- Wi-Fi
const char PAGE[] PROGMEM = R"rawliteral(<!DOCTYPE html><html lang="de"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1"><title>SeedStick</title>
<style>body{font-family:sans-serif;margin:16px;background:#f6f6f2;color:#222}h1{font-size:1.4em;margin:0 0 8px}
.big{font-size:1.6em;font-weight:bold;margin:4px 0}table{border-collapse:collapse;width:100%;font-size:.85em}
td,th{border-bottom:1px solid #ccc;padding:4px;text-align:left}a.b,button{display:inline-block;padding:10px 14px;
margin:6px 6px 6px 0;border:0;border-radius:6px;background:#2e7d32;color:#fff;text-decoration:none;font-size:1em}
button{background:#b23b3b}</style></head><body><h1>SeedStick</h1><div id="st">…</div>
<p><a class="b" href="/log.csv" download="seedstick_log.csv">log.csv herunterladen</a>
<button onclick="del()">Protokoll löschen</button> <a class="b" href="/console">Konsole</a></p><table id="t"></table>
<script>
fetch('/time?t='+Date.now()+'&tz='+new Date().getTimezoneOffset());
function st(){fetch('/status.json').then(r=>r.json()).then(s=>{document.getElementById('st').innerHTML=
'<p class="big">'+s.band+' · '+s.pct+' %</p><p>'+s.count+' Kugeln · Versorgung '+s.supply+' V · seit Start '+
s.uptime+' s</p>';});}
function lg(){fetch('/log.csv').then(r=>r.ok?r.text():'').then(x=>{var h='';x.trim().split('\n').forEach(
function(l,i){if(!l)return;h+='<tr>'+l.split(',').map(function(v){return(i?'<td>':'<th>')+v+(i?'</td>':'</th>');})
.join('')+'</tr>';});document.getElementById('t').innerHTML=h;});}
function del(){if(confirm('Protokoll und Zähler löschen?'))fetch('/clear',{method:'POST'}).then(function(){lg();st();});}
st();lg();setInterval(st,2000);setInterval(lg,10000);
</script></body></html>)rawliteral";

void handleRoot() { server.send(200, "text/html; charset=utf-8", PAGE); }

void handleStatus() {
  char json[160];
  snprintf(json, sizeof json, "{\"band\":\"%s\",\"pct\":%d,\"mv\":%d,\"count\":%lu,\"supply\":\"%.2f\",\"uptime\":%lu}",
           bandTitle(band), moistPct, moistMv, (unsigned long)ballCount, supplyV, (unsigned long)(millis() / 1000));
  server.send(200, "application/json", json);
}

void handleLog() {
  File f;
  if (fsOk && LittleFS.exists(LOG_PATH)) f = LittleFS.open(LOG_PATH, FILE_READ);
  if (!f) {
    server.send(200, "text/csv", "nr,zeit_s,uhrzeit,feuchte_pct,feuchte_mv,band,ergebnis,versorgung_v\n");
    return;
  }
  server.streamFile(f, "text/csv");
  f.close();
}

void handleTime() {
  long long ms = atoll(server.arg("t").c_str());
  if (ms > 1600000000000LL) {
    struct timeval tv;
    tv.tv_sec = (time_t)(ms / 1000);
    tv.tv_usec = (suseconds_t)((ms % 1000) * 1000);
    settimeofday(&tv, nullptr);
    tzOffsetMin = server.arg("tz").toInt();
    timeSet = true;
  }
  server.send(200, "text/plain", "ok");
}

void handleClear() {
  clearLog();
  server.send(200, "text/plain", "ok");
}

const char CONSOLE_PAGE[] PROGMEM = R"rawliteral(<!DOCTYPE html><html lang="de"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1"><title>SeedStick Konsole</title>
<style>body{font-family:sans-serif;margin:12px;background:#f6f6f2;color:#222}h1{font-size:1.3em;margin:0 0 8px}
pre{background:#111;color:#d6f5d6;padding:8px;height:55vh;overflow:auto;font-size:.8em;white-space:pre-wrap;margin:0}
form{display:flex;gap:6px;margin:8px 0}input{flex:1;font-size:1.1em;padding:8px}button{padding:8px 12px;border:0;
border-radius:6px;background:#2e7d32;color:#fff;font-size:1em}.k button{background:#555;margin:3px 3px 3px 0}
a{color:#2e7d32}</style></head><body><h1>SeedStick Konsole</h1><pre id="o"></pre>
<form onsubmit="send(document.getElementById('c').value);document.getElementById('c').value='';return false">
<input id="c" autocomplete="off" autocapitalize="off" placeholder="Befehl, z. B. status"><button>Senden</button></form>
<div class="k"><button onclick="send('status')">status</button><button onclick="send('help')">help</button>
<button onclick="send('a')">a</button><button onclick="send('b')">b</button><button onclick="send('+')">+</button>
<button onclick="send('-')">-</button><button onclick="send('w')">w</button><button onclick="send('o')">o</button>
<button onclick="send('save')">save</button><button onclick="send('test')">test</button>
<button onclick="send('j')">j</button><button onclick="send('')">Enter</button></div>
<p><a href="/">zur Übersicht</a></p>
<script>
var pos=0,o=document.getElementById('o');
function poll(){fetch('/con?from='+pos).then(function(r){pos=parseInt(r.headers.get('X-Pos'))||pos;return r.text();})
.then(function(x){if(x){var end=o.scrollTop+o.clientHeight>=o.scrollHeight-20;o.textContent+=x;
if(o.textContent.length>20000)o.textContent=o.textContent.slice(-15000);if(end)o.scrollTop=o.scrollHeight;}})
.catch(function(){});}
function send(c){fetch('/cmd',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},
body:'c='+encodeURIComponent(c)}).then(poll);}
poll();setInterval(poll,700);
</script></body></html>)rawliteral";

const char CONSOLE_OFF[] PROGMEM = R"rawliteral(<!DOCTYPE html><html lang="de"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1"><title>SeedStick Konsole</title></head>
<body style="font-family:sans-serif;margin:16px"><h1>Konsole aus</h1><p>Die Konsole geht nur mit einem WLAN-Passwort.
Legt neben config.h eine Datei secrets.h an (siehe firmware/README.md) und spielt die Firmware neu auf.</p>
<p><a href="/">zur Übersicht</a></p></body></html>)rawliteral";

void handleConsole() { server.send(200, "text/html; charset=utf-8", consoleAllowed() ? CONSOLE_PAGE : CONSOLE_OFF); }

void handleConOut() {  // text written since ?from=, header X-Pos = new position
  if (!consoleAllowed()) {
    server.send(403, "text/plain", "");
    return;
  }
  uint32_t from = (uint32_t)strtoul(server.arg("from").c_str(), nullptr, 10);
  uint32_t total = conOutTotal;
  if (from > total || total - from > CON_OUT_SIZE) from = total > CON_OUT_SIZE ? total - CON_OUT_SIZE : 0;
  String out;
  out.reserve(total - from);
  for (uint32_t i = from; i < total; i++) out += conOut[i % CON_OUT_SIZE];
  server.sendHeader("X-Pos", String(total));
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "text/plain; charset=utf-8", out);
}

void handleCmd() {  // one line from the page; executed later by handleSerial() in loop(), not here
  if (!consoleAllowed()) {
    server.send(403, "text/plain", "");
    return;
  }
  String c = server.arg("c");
  c.replace("\r", "");
  c.replace("\n", "");
  if (c.length() > 20) c = c.substring(0, 20);
  Con.printf("> %s\n", c.c_str());
  conPush(c.c_str());
  conPush("\n");
  server.send(200, "text/plain", "ok");
}

bool serving = false;
void serveWifi() {  // also called while a command (self-test) waits for input
  if (!wifiOn || serving) return;
  serving = true;
  server.handleClient();
  serving = false;
}

void wifiStart() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(WIFI_SSID, WIFI_PASS);
  WiFi.setTxPower(WIFI_TX_POWER);
  server.begin();
  wifiOn = true;
  showNotice("WLAN an:", WIFI_SSID, "192.168.4.1", 4000);
  Con.println("WLAN an: SeedStick, http://192.168.4.1");
  Con.println(consoleAllowed() ? "Konsole: http://192.168.4.1/console"
                               : "Konsole aus: kein WLAN-Passwort in secrets.h");
}

void wifiStop() {
  server.stop();
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_OFF);
  wifiOn = false;
  showNotice("WLAN aus", "", "", 2000);
  Con.println("WLAN aus");
}

// ---------------------------------------------------------------------------------------------------- LEDs
void updateLeds() {
  uint32_t now = millis();
  bool showsBand = state == State::Ready || state == State::Dose || state == State::Confirmed;
  digitalWrite(PIN_LED_GREEN, showsBand && moistFill > 0 && band == Band::Good);
  bool red = false;
  uint32_t t = now - redSince;
  if (redMode == RedMode::Double) {
    red = t < 150 || (t >= 300 && t < 450);
    if (t >= 600) redMode = RedMode::Off;
  } else if (redMode == RedMode::Fast) {
    red = (t / 100) % 2 == 0;
    if (t >= 3000) redMode = RedMode::Off;
  } else if (state == State::Ready && supplyWarn()) {
    red = now % 2000 < 200;  // slow blink: battery low
  }
  digitalWrite(PIN_LED_RED, red);
}

// ---------------------------------------------------------------------------------------------------- display
void drawCentered(const char* s, int baseline, const uint8_t* font) {
  oled.setFont(font);
  oled.drawUTF8((128 - oled.getUTF8Width(s)) / 2, baseline, s);
}

void drawBig(const char* s, int baseline) {  // at least 24 px where the text fits
  const uint8_t* fonts[] = {u8g2_font_logisoso24_tf, u8g2_font_logisoso20_tf, u8g2_font_logisoso16_tf};
  for (const uint8_t* f : fonts) {
    oled.setFont(f);
    if (oled.getUTF8Width(s) <= 126 || f == fonts[2]) {
      drawCentered(s, baseline, f);
      return;
    }
  }
}

void drawScreen() {
  uint32_t now = millis();
  if (now - lastDrawMs < 100) return;
  lastDrawMs = now;
  oled.clearBuffer();
  char line[32];
  if (pressing && now - pressStart > TAP_MAX_MS && (state == State::Ready || state == State::Calibrate)) {
    uint32_t held = now - pressStart;
    drawCentered("Halten ...", 14, u8g2_font_helvB12_tf);
    uint32_t w = held * 120 / HOLD_CAL_MS;
    if (w > 120) w = 120;
    oled.drawFrame(4, 24, 120, 10);
    oled.drawBox(4, 24, w, 10);
    const char* what = held >= HOLD_CAL_MS ? "loslassen: Kalibrieren"
                     : held >= HOLD_WIFI_MS ? (wifiOn ? "loslassen: WLAN aus" : "loslassen: WLAN an")
                                            : "weiter halten ...";
    drawCentered(what, 52, u8g2_font_6x12_tf);
    oled.sendBuffer();
    return;
  }
  switch (state) {
    case State::Ready:
      if (moistFill == 0) {
        drawBig("...", 27);
      } else {
        drawBig(bandTitle(band), 27);
      }
      snprintf(line, sizeof line, "%d %%  ·  %lu Kugeln", moistPct, (unsigned long)ballCount);
      drawCentered(line, 45, u8g2_font_6x12_tf);
      if (supplyLow()) drawCentered("Akku leer - laden", 61, u8g2_font_6x12_tf);
      else if (supplyWarn()) drawCentered("Akku schwach", 61, u8g2_font_6x12_tf);
      else if (irBlocked) drawCentered("IR-Sensor prüfen", 61, u8g2_font_6x12_tf);
      else if (wifiOn) drawCentered("WLAN: 192.168.4.1", 61, u8g2_font_6x12_tf);
      else drawCentered("anheben, dann tippen", 61, u8g2_font_6x12_tf);
      break;
    case State::Dose:
      drawBig("...", 27);
      drawCentered("Kugel unterwegs", 50, u8g2_font_6x12_tf);
      break;
    case State::Confirmed:
      snprintf(line, sizeof line, "%lu", (unsigned long)ballCount);
      drawBig(line, 27);
      drawCentered("Kugel gesetzt", 50, u8g2_font_6x12_tf);
      break;
    case State::NoBall:
      drawCentered("Keine Kugel", 24, u8g2_font_helvB12_tf);
      drawCentered("Magazin prüfen", 46, u8g2_font_helvB10_tf);
      break;
    case State::Calibrate:
      drawCentered(calStep == 0 ? "Kalibrieren 1/2" : "Kalibrieren 2/2", 13, u8g2_font_helvB10_tf);
      drawCentered(calStep == 0 ? "Sensor in die Luft" : "Sensor ins Wasser", 33, u8g2_font_6x12_tf);
      drawCentered(calStep == 0 ? "halten, dann tippen" : "bis zur Linie, tippen", 47, u8g2_font_6x12_tf);
      snprintf(line, sizeof line, "jetzt %d mV", readMoistureOnce());
      drawCentered(line, 62, u8g2_font_6x12_tf);
      break;
    case State::Notice:
      drawCentered(notice[0], 16, u8g2_font_helvB12_tf);
      drawCentered(notice[1], 38, u8g2_font_6x12_tf);
      drawCentered(notice[2], 54, u8g2_font_6x12_tf);
      break;
  }
  oled.sendBuffer();
}

// ---------------------------------------------------------------------------------------------------- start report
const char* resetReasonText(esp_reset_reason_t r) {
  switch (r) {
    case ESP_RST_POWERON: return "Einschalten oder EN-Taste";
    case ESP_RST_SW: return "Neustart durch Software";
    case ESP_RST_PANIC: return "Absturz";
    case ESP_RST_INT_WDT:
    case ESP_RST_TASK_WDT:
    case ESP_RST_WDT: return "Watchdog";
    case ESP_RST_BROWNOUT: return "Unterspannung";
    case ESP_RST_DEEPSLEEP: return "Tiefschlaf";
    default: return "anderer Grund";
  }
}

void bootReport(esp_reset_reason_t reason) {
  int mv = readMoistureOnce();
  float v = readSupplyNow();
  bool irActive = digitalRead(PIN_IR) == IR_ACTIVE_LEVEL;
  Con.printf("\nSeedStick Firmware %s - Startbericht\n", FW_VERSION);
  Con.printf("  OLED (I2C 0x%02X):      %s\n", OLED_I2C_ADDR,
                oledFound ? "gefunden" : "FEHLT - Kabel SDA 17j, SCK 14j, VDD, GND pruefen (Test t01)");
  Con.printf("  Speicher (LittleFS):  %s, Protokoll %lu Zeilen\n", fsOk ? "ok" : "FEHLER", (unsigned long)logNr);
  Con.printf("  Touch (GPIO %d):       %s\n", PIN_TOUCH,
                digitalRead(PIN_TOUCH) == HIGH ? "beruehrt - beim Einschalten nicht beruehren" : "frei");
  Con.printf("  IR-Sensor (GPIO %d):  OUT %d = %s\n", PIN_IR, digitalRead(PIN_IR), irActive ? "sieht etwas" : "frei");
  Con.printf("  Feuchte (GPIO %d):    %d mV%s\n", PIN_MOIST, mv,
                mv < MOIST_PLAUSIBLE_MIN_MV ? " - kein Signal? (Test t04)" : "");
  Con.printf("  Versorgung (GPIO %d): %.2f V%s\n", PIN_SUPPLY, v,
                v < 1.0f ? " - kein Wert: Teiler R1/R2 und Bruecke B4 pruefen (Test t06)" : "");
  Con.printf("  Servo:                A %d us, B %d us, steht bei %c\n", servoUsA, servoUsB, servoAtA ? 'A' : 'B');
  Con.printf("  Anzeige:              %s\n", oledRot180 ? "um 180 Grad gedreht" : "normal");
  Con.printf("  Neustart-Grund:       %s\n", resetReasonText(reason));
  Con.println("Bereit. \"test\" prueft alle Bauteile nacheinander, \"help\" zeigt alle Befehle.");
}

// ---------------------------------------------------------------------------------------------------- self-test
const char* testWord(TestResult r) {
  switch (r) {
    case TestResult::Ok: return "OK";
    case TestResult::Missing: return "FEHLT";
    case TestResult::Blocked: return "BLOCKIERT";
    case TestResult::Look: return "ansehen";
    default: return "uebersprungen";
  }
}

void testScreen(const char* step, const char* a, const char* b) {
  oled.clearBuffer();
  drawCentered(step, 13, u8g2_font_helvB10_tf);
  drawCentered(a, 36, u8g2_font_6x12_tf);
  drawCentered(b, 52, u8g2_font_6x12_tf);
  oled.sendBuffer();
}

bool enterPressed() {  // Enter in the serial monitor skips the step that is waiting
  bool hit = false;
  while (conAvailable()) {
    if (conRead() == '\n') hit = true;
  }
  return hit;
}

bool askYes(uint32_t ms) {  // "j" + Enter within ms; anything else or nothing = no
  char buf[8];
  size_t len = 0;
  uint32_t t0 = millis();
  while (millis() - t0 < ms) {
    while (conAvailable()) {
      char c = conRead();
      if (c == '\n') return len > 0 && (buf[0] == 'j' || buf[0] == 'J' || buf[0] == 'y' || buf[0] == 'Y');
      if (c != '\r' && len < sizeof buf - 1) buf[len++] = c;
    }
    delay(10);
  }
  return false;
}

TestResult waitTouch() {
  uint32_t t0 = millis();
  while (millis() - t0 < TEST_WAIT_MS) {
    if (digitalRead(PIN_TOUCH) == HIGH) return TestResult::Ok;
    if (enterPressed()) return TestResult::Skipped;
    delay(5);
  }
  return TestResult::Missing;
}

TestResult waitIr() {
  uint32_t edges0 = irEdges;
  uint32_t t0 = millis();
  while (millis() - t0 < TEST_WAIT_MS) {
    if (irEdges != edges0 || digitalRead(PIN_IR) == IR_ACTIVE_LEVEL) return TestResult::Ok;
    if (enterPressed()) return TestResult::Skipped;
    delay(2);
  }
  return TestResult::Missing;
}

void selfTest() {
  static const char* const names[7] = {"OLED", "LEDs", "Touch", "IR-Sensor", "Feuchte", "Versorgung", "Servo"};
  TestResult r[7];
  char line[40];
  servoOff();
  enterPressed();  // drop anything typed after "test"
  Con.printf("\n=== Selbsttest SeedStick %s ===\n", FW_VERSION);
  Con.printf("Jeder Schritt wartet hoechstens %lu s. Enter (oder ein Buchstabe + Enter) ueberspringt ihn.\n",
                (unsigned long)(TEST_WAIT_MS / 1000));

  Con.println("\n1/7 OLED: Auf dem Display steht jetzt \"Selbsttest 1/7\" in einem Rahmen.");
  oled.clearBuffer();
  oled.drawFrame(0, 0, 128, 64);
  drawCentered("Selbsttest 1/7", 26, u8g2_font_helvB10_tf);
  drawCentered("Anzeige ok?", 48, u8g2_font_6x12_tf);
  oled.sendBuffer();
  r[0] = oledFound ? TestResult::Look : TestResult::Missing;
  Con.println(oledFound ? "    I2C antwortet. Seht ihr den Rahmen, ist die Anzeige in Ordnung."
                           : "    FEHLT: keine Antwort an 0x3C. SDA 17j, SCK 14j, VDD, GND pruefen (Test t01).");
  delay(1500);

  Con.println("\n2/7 LEDs: gruen leuchtet 1 s, dann rot 1 s.");
  testScreen("Selbsttest 2/7", "LEDs: erst grün,", "dann rot");
  digitalWrite(PIN_LED_GREEN, HIGH);
  delay(1000);
  digitalWrite(PIN_LED_GREEN, LOW);
  digitalWrite(PIN_LED_RED, HIGH);
  delay(1000);
  digitalWrite(PIN_LED_RED, LOW);
  r[1] = TestResult::Look;
  Con.println("    Leuchteten beide? Sonst: langes Bein = Plus, Anode gruen 23j, rot 22j (Test t02).");

  Con.println("\n3/7 Touch: jetzt das Pad beruehren.");
  testScreen("Selbsttest 3/7", "Touch-Pad", "jetzt berühren");
  r[2] = waitTouch();
  if (r[2] == TestResult::Ok) {
    digitalWrite(PIN_LED_GREEN, HIGH);
    Con.println("    OK: Beruehrung erkannt (gruen leuchtet). Jetzt loslassen.");
    uint32_t t0 = millis();
    while (digitalRead(PIN_TOUCH) == HIGH && millis() - t0 < TEST_WAIT_MS) delay(5);
    digitalWrite(PIN_LED_GREEN, LOW);
  } else if (r[2] == TestResult::Missing) {
    Con.println("    FEHLT: kein Signal an GPIO 4. I/O an 24j, VCC rote, GND blaue Schiene pruefen (Test t02).");
  }

  Con.println("\n4/7 IR-Sensor: Finger, Papier oder Kugel vor das Fenster halten.");
  Con.println("    Ist der Sensor schon im Stab eingebaut: Enter (oder ein Buchstabe + Enter). Dann prueft ihn");
  Con.println("    Schritt 7 mit einer Kugel.");
  testScreen("Selbsttest 4/7", "IR: etwas vor", "das Fenster halten");
  if (digitalRead(PIN_IR) == IR_ACTIVE_LEVEL) {
    r[3] = TestResult::Blocked;
    Con.println("    BLOCKIERT: Der Sensor sieht schon ohne Gegenstand etwas (OUT LOW). Fenster frei? Poti am Modul");
    Con.println("    weniger empfindlich drehen, bis die LED D1 am Modul ausgeht (Test t03).");
  } else {
    r[3] = waitIr();
    if (r[3] == TestResult::Ok) Con.println("    OK: erkannt (die LED D1 am Modul leuchtet dabei).");
    else if (r[3] == TestResult::Missing)
      Con.println("    FEHLT: nichts erkannt. OUT an 19j? Poti am Modul empfindlicher drehen (Test t03).");
  }

  Con.println("\n5/7 Feuchte:");
  int mv = readMoistureOnce();
  int pct = moisturePercent(mv);
  snprintf(line, sizeof line, "%d mV = %d %%", mv, pct);
  testScreen("Selbsttest 5/7", "Feuchte", line);
  if (mv < MOIST_PLAUSIBLE_MIN_MV) {
    r[4] = TestResult::Missing;
    Con.printf("    FEHLT: %d mV, kein Signal. AOUT an 7b, VCC rote, GND blaue Schiene pruefen (Test t04).\n", mv);
  } else if (mv > MOIST_PLAUSIBLE_MAX_MV) {
    r[4] = TestResult::Missing;
    Con.printf("    FEHLT: %d mV, Wert am Anschlag. Verdrahtung pruefen (Test t04).\n", mv);
  } else {
    r[4] = TestResult::Ok;
    Con.printf("    OK: %d mV = %d %% (%s). Im Wasser muss der Wert deutlich sinken.\n", mv, pct,
                  bandName(bandOf(pct)));
  }
  delay(1500);

  Con.println("\n6/7 Versorgung (5V-Knoten):");
  float v = readSupplyNow();
  snprintf(line, sizeof line, "%.2f V", v);
  decimalComma(line);
  testScreen("Selbsttest 6/7", "Versorgung", line);
  if (v < 1.0f) {
    r[5] = TestResult::Missing;
    Con.println("    FEHLT: kein Wert. R1 2c-5c, R2 5a-blau, C2 und Bruecke B4 pruefen (Test t06).");
  } else if (v < SUPPLY_WARN_V) {
    r[5] = TestResult::Look;
    Con.printf("    %.2f V: niedrig. Zellen laden; nur mit USB sind 4,4-5,2 V normal.\n", v);
  } else {
    r[5] = TestResult::Ok;
    Con.printf("    OK: %.2f V.\n", v);
  }
  delay(1500);

  Con.println("\n7/7 Servo: Er faehrt einmal in die andere Endlage, wie beim Dosieren, ohne zu zaehlen.");
  Con.println("    Dafuer braucht er Servostrom: Schalter an, gelber Stecker in 2b (oder die USB-Variante, C9).");
  Con.println("    Liegen Kugeln im Magazin, faellt eine; der IR-Sensor sollte sie sehen.");
  Con.println("    Fahren? \"j\" + Enter; sonst \"n\" + Enter (oder 20 s warten).");
  testScreen("Selbsttest 7/7", "Servo fahren?", "j + Enter am PC");
  if (askYes(20000)) {
    uint32_t edges0 = irEdges;
    uint32_t t0 = millis();
    servoAtA = !servoAtA;
    servoGo(servoAtA ? servoUsA : servoUsB, true);
    prefs.putBool("atA", servoAtA);
    bool ballSeen = false;
    while (millis() - t0 < IR_WINDOW_MS) {
      servoUpdate();
      if (irEdges != edges0 && (int32_t)(irLastEdgeMs - t0) >= (int32_t)IR_IGNORE_MS) ballSeen = true;
      delay(2);
    }
    servoOff();
    r[6] = TestResult::Look;
    Con.printf("    Der Servo steht jetzt bei %c. Hat sich das Rad gedreht? Sonst Servostrom und Stecker pruefen\n",
                  servoAtA ? 'A' : 'B');
    Con.println("    (braun = GND auf dem markierten Pin; Test t05).");
    if (ballSeen) {
      Con.println("    Der IR-Sensor hat dabei eine Kugel gesehen: Dosieren und Bestaetigung funktionieren.");
      if (r[3] != TestResult::Blocked) r[3] = TestResult::Ok;
    }
  } else {
    r[6] = TestResult::Skipped;
  }

  int ok = 0, bad = 0;
  Con.println("\n=== Ergebnis ===");
  for (int i = 0; i < 7; i++) {
    Con.printf("  %-11s %s\n", names[i], testWord(r[i]));
    if (r[i] == TestResult::Ok) ok++;
    if (r[i] == TestResult::Missing || r[i] == TestResult::Blocked) bad++;
  }
  Con.println(bad ? "Bei FEHLT oder BLOCKIERT: den Hinweis oben lesen, dann das genannte Testprogramm."
                     : "Keine Fehler. \"ansehen\" heisst: Das muesst ihr selbst gesehen haben.");
  snprintf(line, sizeof line, "%d OK, %d Fehler", ok, bad);
  showNotice("Selbsttest fertig", line, bad ? "Hinweise am PC" : "", 4000);
  touchRaw = touchStable = digitalRead(PIN_TOUCH) == HIGH;  // touches during the test never count as a tap
  touchRawSince = millis();
  pressStart = millis();
  pressing = false;
  lockoutUntil = millis() + LOCKOUT_MS;
}

// ---------------------------------------------------------------------------------------------------- serial
void printStatus() {
  char volts[12];
  snprintf(volts, sizeof volts, "%.2f", supplyV);
  Con.printf("SeedStick %s | Zustand %d | Kugeln %lu | Protokoll %lu Zeilen | WLAN %s\n", FW_VERSION, (int)state,
                (unsigned long)ballCount, (unsigned long)logNr, wifiOn ? "an" : "aus");
  Con.printf("Feuchte %d mV = %d %% (%s); kalibriert trocken %d mV, nass %d mV\n", moistMv, moistPct,
                bandName(band), moistDryMv, moistWetMv);
  Con.printf("Versorgung %s V | IR OUT %d%s | Servo A %d us, B %d us, steht bei %c\n", volts,
                digitalRead(PIN_IR), irBlocked ? " (blockiert)" : "", servoUsA, servoUsB, servoAtA ? 'A' : 'B');
  Con.printf("OLED %s | Anzeige %s | Touch %s | Speicher %s\n", oledFound ? "gefunden" : "fehlt",
                oledRot180 ? "gedreht" : "normal", digitalRead(PIN_TOUCH) == HIGH ? "beruehrt" : "frei",
                fsOk ? "ok" : "FEHLER");
}

void printHelp() {
  Con.println("Befehle (Zeilenende \"New Line\"):");
  Con.println("  a / b   Servo in Endlage A / B fahren und waehlen (Pulse bleiben an)");
  Con.println("  + / -   gewaehlte Endlage um 10 us aendern");
  Con.println("  w       wechseln wie beim Dosieren (ohne Zaehlen)");
  Con.println("  o       Servo-Pulse aus");
  Con.println("  save    Endlagen A und B speichern");
  Con.println("  status  alle Werte anzeigen");
  Con.println("  cal     Feuchte kalibrieren (dann am Touch tippen)");
  Con.println("  log     Protokoll ausgeben     clear  Protokoll und Zaehler loeschen");
  Con.println("  test    alle Bauteile nacheinander pruefen (Selbsttest)");
  Con.println("  rotate  Anzeige um 180 Grad drehen (wird gespeichert)");
  Con.println("  help    diese Liste");
  Con.println("Web-Konsole: WLAN an (2 s halten), http://192.168.4.1/console (nur mit WLAN-Passwort)");
}

void runCommand(const char* cmd) {
  if (!strcmp(cmd, "a") || !strcmp(cmd, "b")) {
    servoSelected = cmd[0] == 'a' ? 'A' : 'B';
    servoAtA = servoSelected == 'A';
    servoGo(servoAtA ? servoUsA : servoUsB, false);
  } else if (!strcmp(cmd, "+") || !strcmp(cmd, "-")) {
    int step = cmd[0] == '+' ? SERVO_STEP_US : -SERVO_STEP_US;
    int& us = servoSelected == 'A' ? servoUsA : servoUsB;
    us = constrain(us + step, SERVO_US_MIN, SERVO_US_MAX);
    servoAtA = servoSelected == 'A';
    servoGo(us, false);
  } else if (!strcmp(cmd, "w")) {
    servoAtA = !servoAtA;
    servoGo(servoAtA ? servoUsA : servoUsB, true);
  } else if (!strcmp(cmd, "o")) {
    servoOff();
  } else if (!strcmp(cmd, "save")) {
    prefs.putInt("servoA", servoUsA);
    prefs.putInt("servoB", servoUsB);
    prefs.putBool("atA", servoAtA);
    Con.println("Endlagen gespeichert.");
  } else if (!strcmp(cmd, "cal")) {
    startCalibration();
    Con.println("Kalibrieren: Sensor in die Luft halten und am Touch tippen, dann ins Wasser und tippen.");
    return;
  } else if (!strcmp(cmd, "test")) {
    selfTest();
    return;
  } else if (!strcmp(cmd, "rotate")) {
    oledRot180 = !oledRot180;
    prefs.putBool("rot180", oledRot180);
    oled.setDisplayRotation(oledRot180 ? U8G2_R2 : U8G2_R0);
    Con.printf("Anzeige %s (gespeichert).\n", oledRot180 ? "um 180 Grad gedreht" : "wieder normal");
    return;
  } else if (!strcmp(cmd, "log")) {
    File f;
    if (fsOk && LittleFS.exists(LOG_PATH)) f = LittleFS.open(LOG_PATH, FILE_READ);
    if (!f) Con.println("(Protokoll leer)");
    while (f && f.available()) Con.write(f.read());
    if (f) f.close();
    return;
  } else if (!strcmp(cmd, "clear")) {
    clearLog();
    Con.println("Protokoll und Zaehler geloescht.");
    return;
  } else if (!strcmp(cmd, "status")) {
    printStatus();
    return;
  } else {
    printHelp();
    return;
  }
  Con.printf("Servo: A %d us, B %d us, gewaehlt %c%s\n", servoUsA, servoUsB, servoSelected,
                servoActive ? "" : " (aus)");
}

void handleSerial() {
  static char buf[24];
  static uint8_t len = 0;
  while (conAvailable()) {
    char c = conRead();
    if (c == '\r' || c == '\n') {
      if (len) {
        buf[len] = 0;
        runCommand(buf);
        len = 0;
      }
    } else if (len < sizeof buf - 1) {
      buf[len++] = c;
    }
  }
}

// ---------------------------------------------------------------------------------------------------- main
void setup() {
  Serial.begin(115200);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_TOUCH, INPUT_PULLDOWN);  // a loose touch wire reads "not touched", never a tap
  pinMode(PIN_IR, INPUT_PULLUP);       // module missing = "free"; the module's own pull-up stays in parallel
  digitalWrite(PIN_LED_GREEN, HIGH);  // short lamp test
  digitalWrite(PIN_LED_RED, HIGH);

  loadSettings();
  fsOk = LittleFS.begin(true);
  countLogLines();

  Wire.begin(PIN_SDA, PIN_SCL);
  Wire.beginTransmission(OLED_I2C_ADDR);  // does the display answer? (start report, self-test)
  oledFound = Wire.endTransmission() == 0;
  oled.setBusClock(400000);
  oled.begin();
  oled.setDisplayRotation(oledRot180 ? U8G2_R2 : U8G2_R0);
  oled.clearBuffer();
  drawCentered("SeedStick", 30, u8g2_font_logisoso20_tf);
  char ver[24];
  snprintf(ver, sizeof ver, "Firmware %s", FW_VERSION);
  drawCentered(ver, 50, u8g2_font_6x12_tf);
  drawCentered("Touch nicht berühren", 63, u8g2_font_6x12_tf);
  oled.sendBuffer();

  ledcAttach(PIN_SERVO, 50, 14);
  servoGo(servoAtA ? servoUsA : servoUsB, true);  // make sure the wheel stands at its last end position
  attachInterrupt(digitalPinToInterrupt(PIN_IR), onIrEdge, FALLING);

  server.on("/", handleRoot);
  server.on("/status.json", handleStatus);
  server.on("/log.csv", handleLog);
  server.on("/time", handleTime);
  server.on("/clear", HTTP_POST, handleClear);
  server.on("/console", handleConsole);
  server.on("/con", handleConOut);
  server.on("/cmd", HTTP_POST, handleCmd);

  delay(TOUCH_SETTLE_MS);
  digitalWrite(PIN_LED_GREEN, LOW);
  digitalWrite(PIN_LED_RED, LOW);
  setState(State::Ready);

  esp_reset_reason_t reason = esp_reset_reason();
  if (reason == ESP_RST_BROWNOUT) {
    logEntry("neustart_unterspannung");
    showNotice("Neustart:", "Unterspannung", "Akku laden?", 3000);
  }
  if (!oledFound) {  // no display: three long red flashes, so it shows without a PC
    for (int i = 0; i < 3; i++) {
      digitalWrite(PIN_LED_RED, HIGH);
      delay(400);
      digitalWrite(PIN_LED_RED, LOW);
      delay(300);
    }
  }
  bootReport(reason);
}

void loop() {
  uint32_t now = millis();
  servoUpdate();
  updateMoisture();
  updateSupply();
  updateIrBlocked();
  handleSerial();
  serveWifi();

  TouchEvent ev = readTouch();
  switch (state) {
    case State::Ready:
      if (ev == TouchEvent::Tap && (int32_t)(now - lockoutUntil) >= 0) startDose();
      else if (ev == TouchEvent::HoldWifi) wifiOn ? wifiStop() : wifiStart();
      else if (ev == TouchEvent::HoldCal) startCalibration();
      break;
    case State::Dose:
      updateDose();
      break;
    case State::Confirmed:
      if (now - stateSince >= 1500) setState(State::Ready);
      break;
    case State::NoBall:
      if (now - stateSince >= 3000) setState(State::Ready);
      break;
    case State::Calibrate:
      if (ev == TouchEvent::Tap) calibrationTap();
      else if (ev == TouchEvent::HoldWifi || ev == TouchEvent::HoldCal) showNotice("Kalibrieren", "abgebrochen", "", 2000);
      break;
    case State::Notice:
      if ((int32_t)(now - noticeUntil) >= 0) setState(State::Ready);
      break;
  }
  updateLeds();
  drawScreen();
}
