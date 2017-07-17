/*
 * AWS Lambda execution button
 *
 * Created by Sho Kuroda, 2017. (MIT license)
 */

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

const char* SSID = "";
const char* PASSWORD = "";
const char* API_KEY = "";

const char* HOST = "functions.krdlab.com";
const char* FINGERPRINT = "9A E4 50 03 96 39 A0 37 9C 61 BE 9C C5 84 06 95 56";

const int BUTTON = 14;

void setup() {
  Serial.begin(115200);
  Serial.println();

  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println(WiFi.localIP());

  pinMode(BUTTON, INPUT);
}

void loop() {
  const int state1 = digitalRead(BUTTON);
  if (state1 == HIGH) {
    delay(200);
    const int state2 = checkSecondaryClick(BUTTON, 500);
    if (state2 == HIGH) {
      post("/2");
    } else {
      post("/1");
    }
    delay(200);
  }
}

int checkSecondaryClick(const int pin, const unsigned long waitMills) {
  const unsigned long limit = millis() + waitMills;
  int state = LOW;
  while (state == LOW && millis() < limit) {
    state = digitalRead(pin);
    delay(10);
  }
  return state;
}

void post(const char* path) {
  WiFiClientSecure client;
  if (!client.connect(HOST, 443)) {
    Serial.println("failed to connect to host.");
    return;
  }
  if (!client.verify(FINGERPRINT, HOST)) {
    Serial.println("failed to verify fingerprint.");
    // FIXME: return;
  }

  client.print(String("POST ") + path + " HTTP/1.1\r\n"
              + "Host: " + HOST + "\r\n"
              + "Connection: close\r\n"
              + "x-api-key: " + API_KEY + "\r\n"
              + "\r\n");
  Serial.println("POST");

  while (client.connected()) {
    const String line = client.readStringUntil('\n');
    Serial.println(line);
    if (line == "\r") {
      break;
    }
  }
  Serial.println();
  Serial.println("done.");
}
