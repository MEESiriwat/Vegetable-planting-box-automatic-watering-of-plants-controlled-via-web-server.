#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* ssid = "Ita";
const char* password = "sikibidi1";

#define SOIL_SENSOR_PIN A0
#define RELAY_PIN 4
#define SOIL_STATUS_PIN 5

ESP8266WebServer server(80);

bool pumpStatus = false;
bool autoMode = true;

void setup() {
  Serial.begin(115200);

  pinMode(SOIL_SENSOR_PIN, INPUT);
  pinMode(SOIL_STATUS_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  server.on("/", handleRoot);
  server.on("/togglePump", togglePump);
  server.on("/toggleMode", toggleMode);
  server.on("/status", handleStatus);
  server.begin();
}

void loop() {
  server.handleClient();
  int soilMoisture = analogRead(SOIL_SENSOR_PIN);
  int moisturePercent = map(soilMoisture, 1023, 0, 0, 100);
  bool soilStatus = digitalRead(SOIL_STATUS_PIN);

  if (autoMode) {
    if (moisturePercent < 10) {
      digitalWrite(RELAY_PIN, LOW);
    } else if (moisturePercent >= 40) {
      digitalWrite(RELAY_PIN, HIGH);
    }
  }
  pumpStatus = (digitalRead(RELAY_PIN) == HIGH);
  delay(1000);
}

void handleStatus() {
  int soilMoisture = analogRead(SOIL_SENSOR_PIN);
  int moisturePercent = map(soilMoisture, 1023, 0, 0, 100);
  bool soilStatus = digitalRead(SOIL_STATUS_PIN);

  String json = "{";
  json += "\"wifi\":\"" + String(WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected") + "\",";
  json += "\"soil\":\"" + String(soilStatus ? "Dry" : "Wet") + "\",";
  json += "\"moisture\":\"" + String(moisturePercent) + "\",";
  json += "\"pump\":\"" + String(pumpStatus ? "OFF" : "ON") + "\",";
  json += "\"mode\":\"" + String(autoMode ? "Auto" : "Manual") + "\"";
  json += "}";

  server.send(200, "application/json", json);
}


void handleRoot() {
  String html = "<!DOCTYPE html><html lang=\"en\"><head>";
  html += "<meta charset=\"UTF-8\">";
  html += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
  html += "<title>Smart Vegetable Growing Box</title>";
  html += "<style>body{font-family:'Arial',sans-serif;text-align:center;background:#f4f4f4;margin:0;padding:0;display:flex;justify-content:center;align-items:center;min-height:100vh;}";
  html += ".container{background:#ffffff;padding:20px;border-radius:15px;box-shadow:0px 4px 10px rgba(0,0,0,0.2);width:90%;max-width:400px;text-align:center;}";
  html += "h1{color:#2c3e50;font-size:20px;margin-bottom:15px;}";
  html += "p{font-size:16px;color:#333;margin:5px 0;}";
  html += ".progress-container{width:100%;background:#ddd;border-radius:10px;margin:10px 0;height:20px;overflow:hidden;position:relative;}";
  html += ".progress-bar{height:100%;width:0%;background:#28a745;transition:width 0.5s ease-in-out;}";
  html += ".button-group{display:flex;justify-content:space-between;margin-top:15px;}";
  html += "button{flex:1;padding:12px;border:none;border-radius:5px;font-size:14px;color:white;cursor:pointer;transition:background 0.3s ease;margin:5px;}";
  html += ".on{background:#28a745;}.on:hover{background:#218838;}";
  html += ".off{background:#dc3545;}.off:hover{background:#c82333;}";
  html += "@media (max-width:480px){.container{width:95%;max-width:350px;}h1{font-size:18px;}button{font-size:12px;padding:10px;}}";
  html += "</style>";
  html += "<script>function updateStatus(){fetch('/status').then(res=>res.json()).then(data=>{document.getElementById('wifi').innerHTML=data.wifi;document.getElementById('soil').innerHTML=data.soil;document.getElementById('pump').innerHTML=data.pump;document.getElementById('mode').innerHTML=data.mode;document.getElementById('moisture').innerHTML=data.moisture+'%';document.getElementById('bar').style.width=data.moisture+'%';setTimeout(updateStatus,200);});}window.onload=updateStatus;</script>";
  html += "</head><body><div class='container'>";
  html += "<h1>🌱 Smart Vegetable Growing Box 🌿</h1>";
  html += "<p><strong>WiFi Status:</strong> <span id='wifi'>Loading...</span></p>";
  html += "<p><strong>Soil Status:</strong> <span id='soil'>Loading...</span></p>";
  html += "<p><strong>Soil Moisture:</strong> <span id='moisture'>Loading...</span></p>";
  html += "<div class='progress-container'><div id='bar' class='progress-bar'></div></div>";
  html += "<p><strong>Pump Status:</strong> <span id='pump'>Loading...</span></p>";
  html += "<p><strong>Mode:</strong> <span id='mode'>Loading...</span></p>";
  html += "<div class='button-group'>";
  html += "<button class='on' onclick=\"fetch('/togglePump')\">🚰 Toggle Pump</button>";
  html += "<button class='off' onclick=\"fetch('/toggleMode')\">⚙️ Toggle Mode</button>";
  html += "</div></div></body></html>";
  server.send(200, "text/html", html);
}

void toggleMode() {
  autoMode = !autoMode;
  digitalWrite(RELAY_PIN, LOW);
  server.sendHeader("Location", "/", true);
  server.send(303);
}

void togglePump() {
  if (!autoMode) {
    pumpStatus = !pumpStatus;
    digitalWrite(RELAY_PIN, pumpStatus ? HIGH : LOW);
  }
  server.sendHeader("Location", "/", true);
  server.send(303);
}
