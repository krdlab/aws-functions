/*
 * AWS Lambda execution button
 *
 * Created by Sho Kuroda, 2017. (MIT license)
 */

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

const char* SSID = "";
const char* PASSWORD = "";
const char* API_KEY = "";

const char* HOST = "functions.krdlab.com";
const char* FINGERPRINT = "9A E4 50 03 96 39 A0 37 9C 61 BE 9C C5 84 06 95 56";

const int BUTTON = 14;

void setup() {
  Serial.begin(115200);

  Wire.begin(0, 2);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.display();

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

unsigned long startDisplayTime = 0;

void loop() {
  const int state1 = digitalRead(BUTTON);
  if (state1 == HIGH) {
    delay(200);
    const int state2 = checkSecondaryClick(BUTTON, 500);
    if (state2 == HIGH) {
      const String res = post("/2");
      show("taikin", res);
    } else {
      const String res = post("/1");
      show("shukkin", res);
    }
    delay(200);
  }
  if (startDisplayTime > 0 && startDisplayTime + 3000 < millis()) {
    display.clearDisplay();
    display.display();
    startDisplayTime = 0;
  }
}

void show(const char* text, const String tm) {
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.println();
  display.println(text);
  display.println(tm);
  display.display();
  startDisplayTime = millis();
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

String post(const char* path) {
  WiFiClientSecure client;
  if (!client.connect(HOST, 443)) {
    Serial.println("failed to connect to host.");
    return "";
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

  String content = "";
  while (client.connected()) {
    const String line = client.readStringUntil('\n');
    Serial.println(line);
    if (line == "\r") {
      content = client.readStringUntil('\n');
      break;
    }
  }
  Serial.println();
  Serial.println(content);
  Serial.println("done.");
  return content;
}
