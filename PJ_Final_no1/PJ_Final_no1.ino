#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// กำหนดข้อมูล Wi-Fi
const char* ssid = "Ita";
const char* password = "sikibidi1";

// กำหนดพิน
#define SOIL_SENSOR_PIN 5  // D1 = GPIO5 (รับค่าจากเซ็นเซอร์)
#define RELAY_PIN 4        // D2 = GPIO4 (ควบคุมรีเลย์)

// สร้างเว็บเซิร์ฟเวอร์
ESP8266WebServer server(80);

// ตัวแปรเก็บสถานะปั๊มน้ำ
bool pumpStatus = false;

void setup() {
  Serial.begin(115200);

  // ตั้งค่าขา
  pinMode(SOIL_SENSOR_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);  // ปิดปั๊มเริ่มต้น

  // เชื่อมต่อ Wi-Fi
  Serial.print("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);

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

  // ตั้งค่าเว็บเซิร์ฟเวอร์
  server.on("/", handleRoot);
  server.begin();
  Serial.println("Web Server Started.");
}

void loop() {
  server.handleClient();

  // อ่านค่าจากเซ็นเซอร์
  int soilStatus = digitalRead(SOIL_SENSOR_PIN);

  // ถ้าดินแห้ง (ค่าเป็น LOW) ให้เปิดปั๊มน้ำ
  if (soilStatus == LOW) {
    digitalWrite(RELAY_PIN, HIGH);
    pumpStatus = false;
  } else {
    digitalWrite(RELAY_PIN, LOW);
    pumpStatus = true;
  }
}

// ฟังก์ชันสำหรับหน้าเว็บ
void handleRoot() {
  int soilStatus = digitalRead(SOIL_SENSOR_PIN);
  String soilCondition = (soilStatus == HIGH) ? "Dry" : "Wet";
  String pumpState = (pumpStatus) ? "ON" : "OFF";

  // HTML ที่จะส่งกลับ
  String html = "<html>\
    <head>\
      <title>Soil Moisture Monitor</title>\
      <style>body { font-family: Arial; text-align: center; margin-top: 50px; }</style>\
    </head>\
    <body>\
      <h1>Soil Moisture Monitoring</h1>\
      <p>Soil Status: <strong>" + soilCondition + "</strong></p>\
      <p>Pump Status: <strong>" + pumpState + "</strong></p>\
    </body>\
  </html>";

  server.send(200, "text/html", html);
}
