#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <LittleFS.h>
#include <FS.h>
#include <ScoreBoardServer.h>
#include <ScoreBoardState.h>
#include <EspAsyncWiFiManager.h>
#include <ESP8266mDNS.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <qrcode.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define SCREEN_RST_PIN D4

AsyncWebServer server(80);
DNSServer dns;

ScoreBoardStateStore stateStore;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, SCREEN_RST_PIN);

void displayQR(String url)
{
  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(3)];
  qrcode_initText(&qrcode, qrcodeData, 2, ECC_LOW, url.c_str());

  uint8_t scale = 2;                                        // Scale factor for QR code
  uint8_t qrCodeSize = qrcode.size * scale;                 // Size of QR code in pixels
  uint8_t xStart = max(0, (SCREEN_WIDTH - qrCodeSize) / 2); // Calculate starting x-coordinate
  uint8_t yStart = 12;

  for (uint8_t y = 0; y < qrcode.size; y++)
  {
    for (uint8_t x = 0; x < qrcode.size; x++)
    {
      if (qrcode_getModule(&qrcode, x, y))
      {
        display.fillRect(xStart + x * scale, yStart + y * scale, scale, scale, WHITE);
      }
    }
  }
}

void displayCenteredText(String text, bool vertically = false)
{
  int16_t x1;
  int16_t y1;
  uint16_t width;
  uint16_t height;

  display.getTextBounds(text, 0, 0, &x1, &y1, &width, &height);

  if (vertically)
  {
    display.setCursor((SCREEN_WIDTH - width) / 2, (SCREEN_HEIGHT - height) / 2);
  }
  else
  {
    display.setCursor((SCREEN_WIDTH - width) / 2, 0);
  }
  display.println(text);
}

void setup()
{

  Serial.begin(115200);
  Serial.println(F("Starting"));

  Wire.begin(SDA, SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3D))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  displayCenteredText(F(">> SCOREBOARD <<"), true);
  display.display();

  WiFi.mode(WIFI_STA);

  WiFi.hostname(F("ScoreBoard"));

  AsyncWiFiManager wm(&server, &dns);

  bool res;

  res = wm.autoConnect("ScoreBoard Config");
  if (!res)
  {
    Serial.println(F("Failed to connect to WiFi"));
    delay(3000);
    ESP.reset();
  }
  else
  {
    Serial.println(F("Connected to WiFi ") + WiFi.localIP().toString());

    Serial.println(F("Starting mDNS responder..."));
    if (MDNS.begin("board"))
    {
      Serial.println(F("mDNS responder started, hostname: board.local"));
      MDNS.addService(F("http"), F("tcp"), 80);
    }
    else
    {
      Serial.println(F("mDNS responder failed to start!"));
    }

    if (!LittleFS.begin())
    {
      Serial.println(F("An Error has occurred while mounting LittleFS"));
      return;
    }

    Serial.println(F("Staring HTTP server..."));

    ScoreBoardServer scoreBoardServer(&server, &stateStore);
    scoreBoardServer.Start();

    Serial.println(F("HTTP server started"));

    String url = F("http://") + WiFi.localIP().toString();

    display.clearDisplay();
    displayCenteredText(url);
    displayQR(url);
    display.display();
  }
}

void loop()
{
  // put your main code here, to run repeatedly:
}
