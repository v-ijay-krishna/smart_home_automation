#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Servo.h>

// ==========================================
// ⚙️ PIN CONFIGURATION
// ==========================================
const int LIGHT_PIN  = D0; 
const int LCD_SCL_PIN = D1; // Available
const int LCD_SDA_PIN = D2; // Available
const int TEMP_PIN   = A0;  // LM35 Sensor Signal Pin
const int BUZZER_PIN = D4; 
const int ECHO_PIN   = D5; 
const int TRIG_PIN   = D6; 
const int FAN_PIN    = D7; 
const int SERVO_PIN  = D8; 

// ==========================================
// 🛠️ SYSTEM SETTINGS
// ==========================================
Servo doorServo;
const int LOCK_POS = 0;
const int UNLOCK_POS = 170; 
const int DIST_THRESHOLD = 20;

const char* AP_SSID = "TouchlessBell_Cyber"; // Changed AP Name for theme
const char* AP_PASSWORD = "12345678";
const String LOCK_PASSWORD = "admin"; 

ESP8266WebServer server(80);

// Global Variables
bool apActive = false;
bool isLocked = true;
bool lightState = false;
int fanSpeed = 0; 
unsigned long lastTriggerTime = 0;
String systemStatus = "System Online";

float temperatureC = 25.0; // Initialize to reasonable value
unsigned long lastTempReadTime = 0;
const long tempInterval = 5000; // Read temp every 5 seconds

// ==========================================
// 🔊 HELPER FUNCTIONS
// ==========================================

void beep(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(150);
    digitalWrite(BUZZER_PIN, LOW);
    if(i < times -1) delay(100);
  }
}

long getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  if (duration == 0) return 0; 
  return duration * 0.034 / 2;
}

void setFanPwm() {
  int pwm = 0;
  if(fanSpeed == 1) pwm = 400;
  if(fanSpeed == 2) pwm = 700;
  if(fanSpeed == 3) pwm = 1023;
  analogWrite(FAN_PIN, pwm);
}

// LM35 Reading and Conversion
void readTemperature() {
  int adc_val = analogRead(TEMP_PIN);
  float voltage_mV = ( (float)adc_val / 1024.0 ) * 3300.0; // Assuming 3.3V AREF
  float temp_c = voltage_mV / 10.0; 
  
  if (temp_c > -20.0 && temp_c < 100.0) { // Sanity check for LM35 range
    temperatureC = temp_c;
  }
  
  Serial.print("TEMP: "); Serial.print(temperatureC); Serial.println(" C");
}

// ==========================================
// 🌐 UI: LOGIN PAGE (NEON THEME)
// ==========================================
const char PAGE_LOGIN[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>CYBER-SEC LOGIN</title>
  <style>
    @import url('https://fonts.googleapis.com/css2?family=Orbitron:wght@400;700&family=Share+Tech+Mono&display=swap');
    body { 
      background: linear-gradient(135deg, #0a0a0a 0%, #1a0a2a 50%, #0a0a0a 100%); 
      font-family: 'Share Tech Mono', monospace; 
      height: 100vh; display: flex; justify-content: center; align-items: center; 
      margin: 0; color: #00ffcc; text-shadow: 0 0 5px #00ffcc;
      overflow: hidden;
    }
    .card { 
      background: rgba(0, 0, 0, 0.7); 
      backdrop-filter: blur(5px); 
      padding: 40px; 
      border-radius: 15px; 
      border: 2px solid #00ffcc;
      text-align: center; 
      width: 90%; max-width: 350px; 
      box-shadow: 0 0 20px rgba(0, 255, 204, 0.6), inset 0 0 10px rgba(0, 255, 204, 0.4);
      animation: glowPulse 2s infinite alternate;
      position: relative;
    }
    .card::before {
      content: ''; position: absolute; top: -5px; left: -5px; right: -5px; bottom: -5px;
      background: linear-gradient(45deg, #00ffcc, #007bff, #9900ff, #00ffcc);
      z-index: -1; filter: blur(10px); opacity: 0.7;
      animation: borderGlow 4s infinite alternate;
      border-radius: 20px;
    }
    h1 { 
      font-family: 'Orbitron', sans-serif; 
      letter-spacing: 3px; 
      font-weight: 700; 
      color: #00ffcc; 
      text-shadow: 0 0 8px #00ffcc, 0 0 15px #00ffcc;
      margin-bottom: 10px;
    }
    p { 
      color: #00ccff; 
      font-size: 14px; 
      letter-spacing: 2px; 
      margin-top: 0; text-transform: uppercase; 
      text-shadow: 0 0 3px #00ccff;
    }
    input { 
      width: calc(100% - 30px); padding: 15px; margin: 20px 0; 
      background: rgba(10, 20, 30, 0.8); 
      border: 1px solid #00aaff; 
      border-radius: 5px; 
      text-align: center; 
      font-size: 18px; 
      color: #00ffcc; 
      box-sizing: border-box; 
      text-shadow: 0 0 3px #00ffcc;
      box-shadow: 0 0 8px rgba(0, 170, 255, 0.5);
      font-family: 'Share Tech Mono', monospace;
    }
    button { 
      width: 100%; padding: 15px; border: none; border-radius: 5px; 
      background: linear-gradient(90deg, #00ffcc, #007bff); 
      color: #0a0a0a; font-weight: bold; font-size: 16px; 
      cursor: pointer; 
      box-shadow: 0 0 15px rgba(0, 255, 204, 0.7), 0 0 25px rgba(0, 123, 255, 0.5); 
      transition: 0.3s ease-in-out; 
      font-family: 'Orbitron', sans-serif;
      letter-spacing: 1px;
    }
    button:hover { 
      transform: scale(1.03); 
      background: linear-gradient(90deg, #00ffec, #009bff); 
      box-shadow: 0 0 20px rgba(0, 255, 204, 0.9), 0 0 30px rgba(0, 123, 255, 0.7);
    }

    @keyframes glowPulse {
      0% { box-shadow: 0 0 10px rgba(0, 255, 204, 0.4), inset 0 0 5px rgba(0, 255, 204, 0.2); }
      100% { box-shadow: 0 0 25px rgba(0, 255, 204, 0.8), inset 0 0 15px rgba(0, 255, 204, 0.6); }
    }
    @keyframes borderGlow {
      0% { transform: rotate(0deg) scale(1); opacity: 0.7; }
      50% { transform: rotate(180deg) scale(1.02); opacity: 0.9; }
      100% { transform: rotate(360deg) scale(1); opacity: 0.7; }
    }
  </style>
</head>
<body>
  <div class="card">
    <h1>&#128272; CYBER-SEC</h1>
    <p>ACCESS PROTOCOL</p>
    <form action="/login" method="POST">
      <input type="password" name="pwd" placeholder="AUTH KEY" required>
      <button type="submit">INITIATE LOGIN</button>
    </form>
  </div>
</body>
</html>
)=====";

// ==========================================
// 🌐 UI: DASHBOARD (NEON THEME)
// ==========================================
const char PAGE_DASHBOARD[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>CYBER-HOME OS</title>
  <style>
    @import url('https://fonts.googleapis.com/css2?family=Orbitron:wght@400;700&family=Share+Tech+Mono&display=swap');
    body { 
      background: linear-gradient(135deg, #0a0a0a 0%, #1a0a2a 50%, #0a0a0a 100%); 
      color: #00ffcc; 
      font-family: 'Share Tech Mono', monospace; 
      margin: 0; padding: 20px; 
      text-shadow: 0 0 5px #00ffcc;
      overflow-x: hidden;
    }
    .header { 
      font-family: 'Orbitron', sans-serif; 
      font-size: 28px; 
      text-align: center; 
      margin-bottom: 30px; 
      color: #00ccff;
      text-shadow: 0 0 10px #00ccff, 0 0 20px rgba(0, 204, 255, 0.5);
      letter-spacing: 2px;
      animation: headerGlow 3s infinite alternate;
    }
    @keyframes headerGlow {
        0% { text-shadow: 0 0 10px #00ccff, 0 0 20px rgba(0, 204, 255, 0.5); }
        100% { text-shadow: 0 0 15px #00ccff, 0 0 30px rgba(0, 204, 255, 0.8); }
    }
    .status-box { 
      background: rgba(0, 0, 0, 0.7); 
      border: 1px solid #00aaff; 
      padding: 15px; 
      border-radius: 8px; 
      margin-bottom: 25px; 
      color: #00ffcc; 
      font-family: 'Share Tech Mono', monospace; 
      font-size: 14px;
      box-shadow: 0 0 10px rgba(0, 170, 255, 0.5), inset 0 0 5px rgba(0, 170, 255, 0.3);
      text-shadow: 0 0 3px #00ffcc;
    }
    .grid { 
      display: grid; 
      grid-template-columns: repeat(auto-fit, minmax(150px, 1fr)); /* Responsive grid */
      gap: 15px; 
      margin-bottom: 15px;
    }
    .card { 
      background: rgba(10, 10, 20, 0.8); 
      border-radius: 12px; 
      padding: 20px; 
      text-align: center; 
      transition: all 0.3s ease-in-out; 
      border: 1px solid #2a0a4a; /* Violet */
      box-shadow: 0 0 15px rgba(100, 0, 200, 0.5), inset 0 0 8px rgba(100, 0, 200, 0.3);
      cursor: pointer;
    }
    .card:hover { 
      transform: translateY(-5px) scale(1.02); 
      border-color: #00ffcc; /* Green */
      box-shadow: 0 0 20px rgba(0, 255, 204, 0.7), inset 0 0 10px rgba(0, 255, 204, 0.5);
    }
    .icon { 
      font-size: 48px; 
      margin-bottom: 10px; 
      display: block; 
      color: #00ccff; /* Blue */
      text-shadow: 0 0 10px #00ccff, 0 0 20px rgba(0, 204, 255, 0.5);
    }
    .label { 
      font-family: 'Orbitron', sans-serif; 
      font-size: 11px; 
      text-transform: uppercase; 
      color: #888; 
      letter-spacing: 1.5px; 
      margin-bottom: 5px;
    }
    .state { 
      font-family: 'Share Tech Mono', monospace; 
      font-size: 18px; 
      font-weight: bold; 
      margin-top: 5px; 
      color: #00ffcc; 
      text-shadow: 0 0 5px #00ffcc;
    }
    .fan-bar { 
      height: 8px; 
      background: #333; 
      margin-top: 15px; 
      border-radius: 4px; 
      overflow: hidden; 
      box-shadow: inset 0 0 5px rgba(0, 0, 0, 0.5);
    }
    .fan-fill { 
      height: 100%; 
      background: linear-gradient(90deg, #00ffcc, #007bff); 
      width: 0%; 
      transition: width 0.4s ease-out; 
      border-radius: 4px;
      box-shadow: 0 0 10px rgba(0, 255, 204, 0.7);
    }
    .txt-green { color: #00ffcc; text-shadow: 0 0 5px #00ffcc; } 
    .txt-red { color: #ff3399; text-shadow: 0 0 5px #ff3399; } /* Violet-Red for danger */
    .danger { 
      border-color: #ff3399 !important; 
      box-shadow: 0 0 15px rgba(255, 51, 153, 0.7), inset 0 0 8px rgba(255, 51, 153, 0.5); 
    }
    .danger:hover {
        border-color: #ff3399 !important;
        box-shadow: 0 0 25px rgba(255, 51, 153, 0.9), inset 0 0 12px rgba(255, 51, 153, 0.7);
    }
  </style>
</head>
<body>
  <div class="header">CYBER-HOME OS_V3.1</div>
  <div class="status-box" id="console">> STATUS: ONLINE // ALL SYSTEMS NOMINAL</div>
  
  <div class="grid">
    <div class="card" onclick="toggle('lock')">
      <span class="icon" id="lockIcon">&#128274;</span> <div class="label">SECURE.DOOR</div>
      <div class="state txt-red" id="txtLock">LOCKED</div>
    </div>

    <div class="card" onclick="toggle('light')">
      <span class="icon" id="lightIcon">&#128161;</span> <div class="label">ENV.LIGHTING</div>
      <div class="state" id="txtLight">OFF</div>
    </div>

    <div class="card">
      <span class="icon">&#127777;</span> <div class="label">ENV.TEMP</div>
      <div class="state" id="txtTemp">-- &#8451;</div>
    </div>

    <div class="card" onclick="toggle('fan')">
      <span class="icon">&#127744;</span> <div class="label">AIR.FLOW</div>
      <div class="state" id="txtFan">OFF</div>
      <div class="fan-bar"><div class="fan-fill" id="barFan"></div></div>
    </div>
  </div>

  <div class="card danger" style="grid-column: span 2;" onclick="toggle('buzz')">
    <span class="icon">&#128227;</span> <div class="label">WARNING.SYSTEM</div>
    <div class="state txt-red">ACTIVATE ALERT</div>
  </div>

<script>
  function updateUI(data) {
    document.getElementById('console').innerText = "> STATUS: " + data.status.toUpperCase();
    
    // Core Controls
    document.getElementById('lockIcon').innerHTML = data.locked ? "&#128274;" : "&#128275;"; // Locked/Unlocked Icon
    document.getElementById('txtLock').innerText = data.locked ? "LOCKED" : "UNLOCKED";
    document.getElementById('txtLock').className = "state " + (data.locked ? "txt-red" : "txt-green");
    
    document.getElementById('lightIcon').innerHTML = data.light ? "&#128161;" : "&#128161;"; // Same icon, maybe change color
    document.getElementById('txtLight').innerText = data.light ? "ON" : "OFF";
    document.getElementById('txtLight').className = "state " + (data.light ? "txt-green" : "");
    
    // Fan Logic
    let speeds = ["OFF", "LOW", "MED", "HIGH"];
    let widths = ["0%", "33%", "66%", "100%"];
    document.getElementById('txtFan').innerText = speeds[data.fan];
    document.getElementById('barFan').style.width = widths[data.fan];

    // Temperature Display
    document.getElementById('txtTemp').innerText = data.temp.toFixed(1) + " \u00B0C"; // Unicode for Degree symbol
  }
  
  function toggle(type) { 
    fetch('/api?act=' + type).then(r => r.json()).then(d => updateUI(d)); 
  }
  
  // Refresh status every 1 second
  setInterval(() => { fetch('/api?act=status').then(r => r.json()).then(d => updateUI(d)); }, 1000);
</script>
</body></html>
)=====";

// ==========================================
// SERVER LOGIC
// ==========================================

void sendStatus() {
  String json = "{";
  json += "\"temp\":" + String(temperatureC, 1) + ","; 
  json += "\"locked\":" + String(isLocked ? "true" : "false") + ",";
  json += "\"light\":" + String(lightState ? "true" : "false") + ",";
  json += "\"fan\":" + String(fanSpeed) + ",";
  json += "\"status\":\"" + systemStatus + "\"";
  json += "}";
  server.send(200, "application/json", json);
}

void handleAPI() {
  if (!server.hasArg("act")) return;
  String act = server.arg("act");

  if (act == "lock") {
    beep(1); 
    if (isLocked) { doorServo.write(UNLOCK_POS); isLocked = false; systemStatus = "Door Unlocked. Access Granted."; }
    else { doorServo.write(LOCK_POS); isLocked = true; systemStatus = "Door Locked. Perimeter Secure."; }
  }
  else if (act == "light") {
    beep(1); 
    lightState = !lightState;
    digitalWrite(LIGHT_PIN, lightState ? HIGH : LOW);
    systemStatus = lightState ? "Lighting Activated." : "Lighting Deactivated.";
  }
  else if (act == "fan") {
    beep(1); 
    fanSpeed++; if (fanSpeed > 3) fanSpeed = 0;
    setFanPwm();
    systemStatus = "Airflow Set to Level " + String(fanSpeed) + ".";
  }
  else if (act == "buzz") {
    systemStatus = "WARNING: Alert System Activated!";
    digitalWrite(BUZZER_PIN, HIGH); delay(300);
    digitalWrite(BUZZER_PIN, LOW); delay(100);
    digitalWrite(BUZZER_PIN, HIGH); delay(300);
    digitalWrite(BUZZER_PIN, LOW);
  }
  
  sendStatus();
}

void handleLogin() {
  if (server.arg("pwd") == LOCK_PASSWORD) {
    doorServo.write(UNLOCK_POS);
    isLocked = false;
    systemStatus = "Identity Verified. Access Granted.";
    server.sendHeader("Location", "/dashboard");
    server.send(303);
  } else {
    beep(2);
    systemStatus = "Authentication Failed. Access Denied.";
    server.send(200, "text/html", "<script>alert('ACCESS DENIED. INCORRECT AUTH KEY.'); window.location='/';</script>");
  }
}

void startEverything() {
  if(apActive) return;
  
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  
  server.on("/", [](){ server.send(200, "text/html", PAGE_LOGIN); });
  server.on("/login", handleLogin);
  server.on("/dashboard", [](){ server.send(200, "text/html", PAGE_DASHBOARD); });
  server.on("/api", handleAPI); 
  
  server.begin();
  apActive = true;
}

// ==========================================
// SETUP & LOOP
// ==========================================
void setup() {
  Serial.begin(115200);
  
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LIGHT_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  
  digitalWrite(LIGHT_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  analogWrite(FAN_PIN, 0);

  doorServo.attach(SERVO_PIN);
  doorServo.write(LOCK_POS); 

  readTemperature(); // Initial read

  startEverything();
  
  Serial.println("System Initialized. Awaiting Protocol...");
  Serial.print("✅ IP: "); Serial.println(WiFi.softAPIP());
  beep(1);
}

void loop() {
  if (apActive) server.handleClient();

  if (millis() - lastTempReadTime >= tempInterval) {
    readTemperature();
    lastTempReadTime = millis();
  }

  long dist = getDistance();

  if (dist > 0 && dist < DIST_THRESHOLD) {
    if (millis() - lastTriggerTime > 3000) {
      beep(2);
      systemStatus = "INTRUDER ALERT! Perimeter Breach Detected!";
      lastTriggerTime = millis();
    }
  }
  delay(100);
}
