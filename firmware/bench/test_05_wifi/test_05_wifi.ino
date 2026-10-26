// join wifi and serve a page. edit the two credentials below, don't commit them.
// pass: serial prints an IP, opening it on the same network toggles the led.
// 2.4 GHz only, and eduroam won't work.

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
