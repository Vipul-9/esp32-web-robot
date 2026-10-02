/*
 * Web-Controlled Mobile Robot (ESP32)
 *
 * The ESP32 hosts a control page. Hold a direction button in the browser to
 * drive; release to stop. A failsafe stops the motors if no command arrives
 * for 500 ms (e.g. Wi-Fi drops). Speed is set with a slider.
 *
 * Board:  ESP32 Dev Module (Arduino-ESP32 core 2.0.11 or newer)
 * Driver: L298N dual H-bridge
 * Wiring:
 *   ENA -> GPIO 14   IN1 -> GPIO 27   IN2 -> GPIO 26   (left motor)
 *   ENB -> GPIO 32   IN3 -> GPIO 25   IN4 -> GPIO 33   (right motor)
 *   L298N GND <-> ESP32 GND, motor battery -> L298N 12V input
 *   (remove the ENA/ENB jumpers so the ESP32 controls speed)
 *
 * Wi-Fi: joins your network if WIFI_SSID is set; otherwise (or if it fails)
 * starts its own hotspot "ESP32-Robot"; open http://192.168.4.1
 */

#include <WiFi.h>
#include <WebServer.h>

// ---------------- User configuration ----------------
const char *WIFI_SSID = "";              // leave empty to always use hotspot mode
const char *WIFI_PASS = "";
const char *AP_SSID   = "ESP32-Robot";
const char *AP_PASS   = "robot1234";     // min 8 characters
// ----------------------------------------------------

const int ENA = 14, IN1 = 27, IN2 = 26;
const int ENB = 32, IN3 = 25, IN4 = 33;
const unsigned long FAILSAFE_MS = 500;

WebServer server(80);
int speedValue = 200;                    // 0-255
unsigned long lastCommand = 0;
bool moving = false;

const char PAGE[] PROGMEM = R"HTML(
<!DOCTYPE html><html><head>
<meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">
<title>ESP32 Robot</title>
<style>
 body{font-family:sans-serif;text-align:center;background:#111;color:#eee;margin:0;padding:20px;user-select:none}
 h2{margin:8px 0 20px}
 .pad{display:grid;grid-template-columns:repeat(3,90px);grid-gap:10px;justify-content:center}
 button{height:90px;font-size:30px;border:none;border-radius:14px;background:#2d6cdf;color:#fff;touch-action:none}
 button:active,button.on{background:#1b4aa3}
 .stop{background:#c0392b}
 input{width:260px;margin-top:24px}
</style></head><body>
<h2>ESP32 Robot</h2>
<div class="pad">
 <span></span><button data-d="F">&#9650;</button><span></span>
 <button data-d="L">&#9664;</button><button class="stop" data-d="S">&#9632;</button><button data-d="R">&#9654;</button>
 <span></span><button data-d="B">&#9660;</button><span></span>
</div>
<div>Speed: <span id="sv">200</span></div>
<input type="range" min="80" max="255" value="200" id="spd">
<script>
let timer=null;
function send(d){fetch('/cmd?d='+d).catch(()=>{});}
function start(d,b){stop();b.classList.add('on');send(d);
  if(d!=='S') timer=setInterval(()=>send(d),200);}   // keep-alive for failsafe
function stop(){if(timer){clearInterval(timer);timer=null;}
  document.querySelectorAll('button').forEach(x=>x.classList.remove('on'));}
document.querySelectorAll('button').forEach(b=>{
  const d=b.dataset.d;
  b.addEventListener('pointerdown',e=>{e.preventDefault();start(d,b);});
  ['pointerup','pointerleave','pointercancel'].forEach(ev=>
    b.addEventListener(ev,()=>{if(timer){stop();send('S');}}));
});
const s=document.getElementById('spd');
s.oninput=()=>{document.getElementById('sv').textContent=s.value;};
s.onchange=()=>fetch('/speed?v='+s.value).catch(()=>{});
document.addEventListener('keydown',e=>{const m={ArrowUp:'F',ArrowDown:'B',ArrowLeft:'L',ArrowRight:'R'};
  if(m[e.key]&&!e.repeat){const b=document.querySelector('[data-d='+m[e.key]+']');start(m[e.key],b);}});
document.addEventListener('keyup',e=>{if(['ArrowUp','ArrowDown','ArrowLeft','ArrowRight'].includes(e.key)){stop();send('S');}});
</script></body></html>
)HTML";

// ---------------- Motors ----------------
void setMotor(int en, int inA, int inB, int dir) {   // dir: 1 fwd, -1 rev, 0 stop
  digitalWrite(inA, dir > 0 ? HIGH : LOW);
  digitalWrite(inB, dir < 0 ? HIGH : LOW);
  analogWrite(en, dir == 0 ? 0 : speedValue);
}

void drive(char d) {
  switch (d) {
    case 'F': setMotor(ENA, IN1, IN2,  1); setMotor(ENB, IN3, IN4,  1); break;
    case 'B': setMotor(ENA, IN1, IN2, -1); setMotor(ENB, IN3, IN4, -1); break;
    case 'L': setMotor(ENA, IN1, IN2, -1); setMotor(ENB, IN3, IN4,  1); break;  // spin left
    case 'R': setMotor(ENA, IN1, IN2,  1); setMotor(ENB, IN3, IN4, -1); break;  // spin right
    default:  setMotor(ENA, IN1, IN2,  0); setMotor(ENB, IN3, IN4,  0); break;
  }
  moving = (d == 'F' || d == 'B' || d == 'L' || d == 'R');
}

// ---------------- Web handlers ----------------
void handleRoot() { server.send_P(200, "text/html", PAGE); }

void handleCmd() {
  String d = server.arg("d");
  if (d.length() == 1 && strchr("FBLRS", d[0])) {
    drive(d[0]);
    lastCommand = millis();
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "bad command");
  }
}

void handleSpeed() {
  speedValue = constrain(server.arg("v").toInt(), 0, 255);
  server.send(200, "text/plain", String(speedValue));
}

// ---------------- Wi-Fi ----------------
void startWiFi() {
  if (strlen(WIFI_SSID) > 0) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.print("Connecting to Wi-Fi");
    for (int i = 0; i < 20 && WiFi.status() != WL_CONNECTED; i++) { delay(500); Serial.print('.'); }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.print("\nOpen http://"); Serial.println(WiFi.localIP());
      return;
    }
    Serial.println("\nWi-Fi failed, starting hotspot");
  }
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASS);
  Serial.print("Hotspot "); Serial.print(AP_SSID);
  Serial.print(" - open http://"); Serial.println(WiFi.softAPIP());
}

void setup() {
  Serial.begin(115200);
  int pins[] = {ENA, IN1, IN2, ENB, IN3, IN4};
  for (int p : pins) pinMode(p, OUTPUT);
  drive('S');

  startWiFi();
  server.on("/", handleRoot);
  server.on("/cmd", handleCmd);
  server.on("/speed", handleSpeed);
  server.begin();
}

void loop() {
  server.handleClient();
  if (moving && millis() - lastCommand > FAILSAFE_MS) drive('S');   // failsafe stop
}
