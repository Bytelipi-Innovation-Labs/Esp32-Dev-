
#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <U8g2lib.h>

// =====================================
// OLED CONFIGURATION
// =====================================

#define OLED_SDA 18
#define OLED_SCL 19

// SSD1306 128x64 I2C OLED
U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled(
  U8G2_R0,
  U8X8_PIN_NONE
);

// =====================================
// WIFI ACCESS POINT CONFIGURATION
// =====================================

const char* AP_SSID = "ESP32-OLED";
const char* AP_PASSWORD = "12345678";

WebServer server(80);

// Message shown on OLED
String oledMessage = "Hello ESP32!";

// =====================================
// WEB DASHBOARD HTML
// =====================================

const char webpage[] PROGMEM = R"rawliteral(

<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">

<title>ESP32 OLED Dashboard</title>

<style>

body {
  font-family: Arial, sans-serif;
  background: #101827;
  color: white;
  text-align: center;
  padding: 20px;
}

.container {
  max-width: 450px;
  margin: auto;
  background: #1e293b;
  padding: 25px;
  border-radius: 15px;
}

h2 {
  color: #38bdf8;
}

textarea {
  width: 95%;
  height: 120px;
  padding: 10px;
  font-size: 18px;
  border-radius: 8px;
  border: none;
  resize: vertical;
}

button {
  width: 100%;
  padding: 15px;
  margin-top: 15px;
  background: #0ea5e9;
  color: white;
  border: none;
  border-radius: 8px;
  font-size: 18px;
  cursor: pointer;
}

button:hover {
  background: #0284c7;
}

.info {
  color: #94a3b8;
  margin-top: 15px;
}

#status {
  margin-top: 15px;
  color: #4ade80;
}

</style>
</head>

<body>

<div class="container">

<h2>ESP32 OLED Dashboard</h2>

<p>Type a message to display on your OLED</p>

<textarea id="message" maxlength="300"
placeholder="Enter your message..."></textarea>

<button onclick="sendMessage()">
Display on OLED
</button>

<button onclick="clearOLED()">
Clear OLED
</button>

<div id="status">Connected to dashboard</div>

<div class="info">
WiFi: ESP32-OLED<br>
IP Address: 192.168.4.1
</div>

</div>

<script>

function sendMessage() {

  let message = document.getElementById("message").value;

  fetch("/display", {
    method: "POST",
    headers: {
      "Content-Type": "text/plain"
    },
    body: message
  })
  .then(response => response.text())
  .then(data => {
    document.getElementById("status").innerText = data;
  })
  .catch(error => {
    document.getElementById("status").innerText =
      "Connection error";
  });

}

function clearOLED() {

  document.getElementById("message").value = "";

  fetch("/clear", {
    method: "POST"
  })
  .then(response => response.text())
  .then(data => {
    document.getElementById("status").innerText = data;
  });

}

</script>

</body>
</html>

)rawliteral";

// =====================================
// DISPLAY MESSAGE ON OLED
// =====================================

void displayMessage(String message) {

  oled.clearBuffer();

  oled.setFont(u8g2_font_6x12_tf);

  String lines[5];

  int lineCount = 0;

  String currentLine = "";

  for (int i = 0; i < message.length(); i++) {

    char c = message[i];

    if (c == '\n') {

      if (lineCount < 5) {
        lines[lineCount++] = currentLine;
      }

      currentLine = "";

      continue;
    }

    String testLine = currentLine + c;

    if (oled.getStrWidth(testLine.c_str()) > 126) {

      if (lineCount < 5) {
        lines[lineCount++] = currentLine;
      }

      currentLine = "";

      if (lineCount >= 5) {
        break;
      }

    }

    currentLine += c;
  }

  if (lineCount < 5 && currentLine.length() > 0) {
    lines[lineCount++] = currentLine;
  }

  // Draw text on OLED
  for (int i = 0; i < lineCount; i++) {

    oled.drawStr(
      0,
      12 + (i * 12),
      lines[i].c_str()
    );

  }

  oled.sendBuffer();
}

// =====================================
// WEB SERVER HANDLERS
// =====================================

void handleRoot() {

  server.send_P(
    200,
    "text/html",
    webpage
  );

}

void handleDisplay() {

  if (server.hasArg("plain")) {

    oledMessage = server.arg("plain");

    displayMessage(oledMessage);

    server.send(
      200,
      "text/plain",
      "Message displayed on OLED!"
    );

  } else {

    server.send(
      400,
      "text/plain",
      "No message received"
    );

  }

}

void handleClear() {

  oledMessage = "";

  oled.clearBuffer();
  oled.sendBuffer();

  server.send(
    200,
    "text/plain",
    "OLED cleared!"
  );

}

// =====================================
// SETUP
// =====================================

void setup() {

  Serial.begin(115200);

  // Initialize OLED I2C
  Wire.begin(OLED_SDA, OLED_SCL);

  oled.setI2CAddress(0x3C * 2);

  oled.begin();

  oled.clearBuffer();

  oled.setFont(u8g2_font_6x12_tf);

  oled.drawStr(0, 15, "ESP32 OLED");
  oled.drawStr(0, 30, "Starting WiFi AP...");
  oled.sendBuffer();

  // Start WiFi Access Point
  WiFi.mode(WIFI_AP);

  bool apStarted = WiFi.softAP(
    AP_SSID,
    AP_PASSWORD,
    6,
    false,
    4
  );

  if (apStarted) {

    Serial.println("WiFi AP Started!");

    Serial.print("SSID: ");
    Serial.println(AP_SSID);

    Serial.print("Password: ");
    Serial.println(AP_PASSWORD);

    Serial.print("IP Address: ");
    Serial.println(WiFi.softAPIP());

  } else {

    Serial.println("AP Start Failed!");

  }

  // Display AP information
  oled.clearBuffer();

  oled.drawStr(0, 12, "ESP32 OLED AP");

  oled.drawStr(0, 28, "WiFi: ESP32-OLED");

  oled.drawStr(0, 44, "IP: 192.168.4.1");

  oled.drawStr(0, 60, "Dashboard Ready");

  oled.sendBuffer();

  // Configure web routes
  server.on("/", HTTP_GET, handleRoot);

  server.on(
    "/display",
    HTTP_POST,
    handleDisplay
  );

  server.on(
    "/clear",
    HTTP_POST,
    handleClear
  );

  // Start web server
  server.begin();

  Serial.println("Web Server Started!");
  Serial.println("Open http://192.168.4.1");
}

// =====================================
// LOOP
// =====================================

void loop() {

  server.handleClient();

}
