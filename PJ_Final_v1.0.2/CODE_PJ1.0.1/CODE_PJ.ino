#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// ข้อมูล Wi-Fi
const char* ssid = "Ita";
const char* password = "sikibidi1";

// กำหนดขา
#define SOIL_SENSOR_PIN 5  // D1 = GPIO5 (รับค่าความชื้น)
#define RELAY_PIN 4        // D2 = GPIO4 (ควบคุมปั๊มน้ำ)

// สร้างเว็บเซิร์ฟเวอร์
ESP8266WebServer server(80);

// ตัวแปรควบคุมระบบ
bool pumpStatus = false;   // สถานะปั๊ม
bool autoMode = true;      // โหมดอัตโนมัติ

void setup() {
  Serial.begin(115200);

  // ตั้งค่าขา
  pinMode(SOIL_SENSOR_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);  // ปิดปั๊มเริ่มต้น

  // เชื่อมต่อ Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi...");
  int timeout = 20;
  while (WiFi.status() != WL_CONNECTED && timeout > 0) {
    delay(500);
    Serial.print(".");
    timeout--;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWi-Fi connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFailed to connect to Wi-Fi.");
  }

  // ตั้งค่าหน้าเว็บ
  server.on("/", handleRoot);
  server.on("/togglePump", togglePump);
  server.on("/toggleMode", toggleMode);
  server.on("/getStatus", sendStatus);

  server.begin();
  Serial.println("Web Server Started.");
}

void loop() {
  server.handleClient();

  // อ่านค่าความชื้นดิน
  int soilStatus = digitalRead(SOIL_SENSOR_PIN);

  // ทำงานในโหมดอัตโนมัติ
  if (autoMode) {
    if (soilStatus == LOW) {  // ดินแห้ง
      digitalWrite(RELAY_PIN, HIGH);
    } else {  // ดินเปียก
      digitalWrite(RELAY_PIN, LOW);
    }
  }

  // อัปเดตสถานะปั๊มน้ำตามรีเลย์จริง
  pumpStatus = (digitalRead(RELAY_PIN) == HIGH);
}

// ฟังก์ชันสำหรับหน้าเว็บหลัก
void handleRoot() {
  String html = "<html>\
    <head>\
      <title>Soil Moisture Monitor</title>\
      <style>\
        body { font-family: Arial, sans-serif; text-align: center; background: #f4f4f4; margin-top: 50px; }\
        .container { background: white; padding: 20px; border-radius: 10px; box-shadow: 0 0 10px rgba(0, 0, 0, 0.1); display: inline-block; }\
        h1 { color: #333; }\
        p { font-size: 20px; }\
        .status { font-size: 24px; font-weight: bold; }\
        .button { padding: 10px 20px; font-size: 18px; margin-top: 10px; cursor: pointer; border: none; border-radius: 5px; color: white; }\
        .on { background: #28a745; }\
        .off { background: #dc3545; }\
        .auto { background: #007bff; }\
        .manual { background: #ffc107; color: black; }\
      </style>\
      <script>\
        function sendRequest(url) {\
          fetch(url).then(() => updateStatus());\
        }\
        function updateStatus() {\
          fetch('/getStatus').then(response => response.json()).then(data => {\
            document.getElementById('soilStatus').innerText = data.soil;\
            document.getElementById('pumpStatus').innerText = data.pump;\
            document.getElementById('modeStatus').innerText = data.mode;\
            document.getElementById('modeButton').innerText = (data.mode === 'Auto') ? 'Switch to Manual' : 'Switch to Auto';\
            document.getElementById('modeButton').className = 'button ' + ((data.mode === 'Auto') ? 'manual' : 'auto');\
            document.getElementById('pumpButton').innerText = (data.pump === 'ON') ? 'Turn Off' : 'Turn On';\
            document.getElementById('pumpButton').className = 'button ' + ((data.pump === 'ON') ? 'on' : 'off');\
            document.getElementById('pumpButton').disabled = (data.mode === 'Auto');\
          });\
        }\
        setInterval(updateStatus, 2000);\
      </script>\
    </head>\
    <body onload='updateStatus()'>\
      <div class='container'>\
        <h1>Soil Moisture Monitoring</h1>\
        <p>Soil Status: <span id='soilStatus' class='status'>Loading...</span></p>\
        <p>Pump Status: <span id='pumpStatus' class='status'>Loading...</span></p>\
        <p>Mode: <span id='modeStatus' class='status'>Loading...</span></p>\
        <button id='modeButton' class='button' onclick=\"sendRequest('/toggleMode')\">Loading...</button>\
        <button id='pumpButton' class='button' onclick=\"sendRequest('/togglePump')\" disabled>Loading...</button>\
      </div>\
    </body>\
  </html>";

  server.send(200, "text/html", html);
}

// ฟังก์ชันสลับโหมด Auto/Manual
void toggleMode() {
  autoMode = !autoMode;
  if (autoMode) {
    digitalWrite(RELAY_PIN, LOW);  // ปิดปั๊มเมื่อเปลี่ยนไปโหมด Auto
  }
  server.send(200, "text/plain", "Mode Changed");
}

// ฟังก์ชันเปิด-ปิดปั๊มน้ำแบบ Manual
void togglePump() {
  if (!autoMode) {  // ควบคุมปั๊มได้เฉพาะโหมด Manual
    pumpStatus = !pumpStatus;
    digitalWrite(RELAY_PIN, pumpStatus ? HIGH : LOW);
  }
  server.send(200, "text/plain", "Pump Toggled");
}

// ฟังก์ชันส่งข้อมูลสถานะไปยังหน้าเว็บ
void sendStatus() {
  int soilStatus = digitalRead(SOIL_SENSOR_PIN);
  String soilCondition = (soilStatus == HIGH) ? "Dry" : "Wet";
  String pumpState = (digitalRead(RELAY_PIN) == LOW) ? "ON" : "OFF";  // แก้ให้สถานะถูกต้อง
  String modeState = (autoMode) ? "Auto" : "Manual";

  String json = "{\"soil\":\"" + soilCondition + "\",\"pump\":\"" + pumpState + "\",\"mode\":\"" + modeState + "\"}";
  server.send(200, "application/json", json);
}
