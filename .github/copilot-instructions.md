# Copilot Instructions

## Project Overview

An Arduino/C++ firmware project for ESP8266 (d1_mini) and ESP32-S3 that controls a physical baseball scoreboard. The device:
- Connects to WiFi via captive portal (ESPAsyncWiFiManager)
- Serves a pre-built Angular web app from LittleFS over HTTP (ESPAsyncWebServer)
- Communicates with scoreboard hardware over SoftwareSerial (pins D5/D6)
- Shows the device IP and QR code on a 128×64 OLED display (SSD1306)
- Advertises itself as `board.local` via mDNS

## Build Commands (PlatformIO)

```sh
pio run                              # Build (default env)
pio run -e d1_mini                   # Build for ESP8266
pio run -e esp32-s3                  # Build for ESP32-S3
pio run -e d1_mini --target upload   # Flash firmware
pio run -e d1_mini --target uploadfs # Upload web frontend to LittleFS
pio device monitor --baud 115200     # Serial monitor
pio test                             # Run unit tests
```

## Architecture

```
src/main.cpp          – Entry point: WiFi, OLED, mDNS, wires together ScoreBoardServer + ScoreBoardStateStore
lib/core/
  ScoreBoardCom       – SoftwareSerial wrapper; sends text commands, looks for expected string in response
  ScoreBoardState     – State struct (home, away, inning, outsInInning) + ScoreBoardStateStore that maps
                        state fields to hardware commands and only persists state after successful write
  ScoreBoardServer    – ESPAsyncWebServer routes; serves static files from LittleFS and REST API
data/browser/         – Pre-built Angular app uploaded to LittleFS; DO NOT hand-edit these files
```

## REST API

| Method | Path | Description |
|--------|------|-------------|
| GET | `/api/score-board/load` | Returns current state as JSON |
| POST | `/api/score-board/update` | Updates scoreboard; body: `{"payload":{"home":0,"away":0,"inning":1,"outsInInning":0}}` |
| GET | `/api/score-board/status` | Returns `{"isReady": true/false}` |
| GET | `/api/score-board/start` | Opens serial connection and returns ready status |

## Hardware Communication Protocol

`ScoreBoardCom` communicates at 9600 baud over SoftwareSerial. Commands are newline-terminated strings:

- `wd <register> <digit>` — write a single digit to a display position
- `wb <register> <bit> <value>` — write a single bit (LED on/off)

Register mapping used in `ScoreBoardState::UpdateAll`:

| Register | Meaning |
|----------|---------|
| 0 | Home score — tens digit |
| 1 | Home score — ones digit |
| 2 | Inning |
| 3 | Away score — tens digit |
| 4 | Away score — ones digit |
| 5, bit 0 | First out LED |
| 5, bit 1 | Second out LED |

A successful command returns a response containing `"Ok"`. Connection health is checked by sending `"C"` or `"version"` and looking for `"Control SW"`.

## Key Conventions

- Use the `F()` macro for all string literals passed to Serial/display to store them in flash and save heap RAM.
- `ScoreBoardStateStore::updateState()` only updates the in-memory state if all hardware writes succeed — never update `scoreBoardState` directly.
- The `com` object is owned by `ScoreBoardStateStore` via `std::unique_ptr`; `ScoreBoardServer` receives a raw pointer to the store.
- The web frontend is a pre-built bundle in `data/browser/`. To change the UI, build the Angular project separately and copy the output there, then re-upload with `uploadfs`.
- Both ESP8266 and ESP32-S3 environments share the same source; keep platform-specific code behind `#ifdef` guards if needed.
