// Bench test 5: can the ESP32 join WiFi and serve a page?
// Needs: ESP32 on USB only. Edit the two WiFi lines below first.
// PASS: Serial Monitor prints an IP address; opening that address on a
//       phone or laptop on the same WiFi shows a message and toggles the LED.

#include <WiFi.h>
#include <WebServer.h>

const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
#define LED_PIN 2

WebServer server(80);
bool ledOn = false;

void handleRoot() {
  ledOn = !ledOn;
  digitalWrite(LED_PIN, ledOn);
  server.send(200, "text/plain", ledOn ? "Rover bench test: LED ON" : "Rover bench test: LED OFF");
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println();
  Serial.print("Open http://");
  Serial.println(WiFi.localIP());
  server.on("/", handleRoot);
  server.begin();
}

void loop() {
  server.handleClient();
}
