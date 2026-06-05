# lora_ukfe — USB Army Knife Flipper Edition

> Flipper Zero FAP · Remote-Steuerung für den Heltec ESP32 LoRa v3 Agent via UART/JSON

![Version](https://img.shields.io/badge/version-1.0.0-blue)
![API](https://img.shields.io/badge/Flipper%20API-87.1-orange)
![SDK](https://img.shields.io/badge/SDK-mntm--012-purple)
![Category](https://img.shields.io/badge/category-GPIO-green)
![License](https://img.shields.io/badge/license-GPL--3.0-red)

---

## Übersicht

`lora_ukfe` ist eine Flipper Zero App (`.fap`), die als Fernbedienung für den **USB Army Knife Flipper Edition (UKFE)** Agenten auf einem Heltec ESP32 LoRa v3 dient. Die Kommunikation erfolgt über UART (115200 Baud) mit einem JSON-Protokoll.

### Features

| Funktion | Beschreibung |
|---|---|
| **Status** | Live-Status vom Heltec Agent (State, RSSI, WiFi-Clients, Battery) |
| **Trigger** | BadUSB/HID-Payloads remote auslösen (mit optionalem Delay) |
| **Payload-Liste** | Alle auf dem Heltec gespeicherten Payloads anzeigen |
| **LoRa Scan** | Umgebungs-Scan auf 868 MHz EU-Band |
| **WiFi Scan** | SSID-Scan über ESP32 WiFi |
| **WiFi Deauth** | Deauth-Frame-Sender (nur für autorisierte Pentests) |
| **Evil Portal** | Captive Portal starten/stoppen |
| **ABORT** | Sofortabbruch aller laufenden Aktionen |
| **Log** | JSON-Echtzeit-Log der Heltec-Kommunikation |
| **Einstellungen** | Modus (LoRa / WiFi / SubGHz / Direct), Baud-Rate |

---

## Architektur

```
Flipper Zero                         Heltec ESP32 LoRa v3
┌─────────────────────────────┐      ┌──────────────────────────────┐
│  lora_ukfe.fap              │      │  G4MEOVER LoRa Agent v1.0    │
│                             │      │                              │
│  ┌──────────┐  JSON-Cmds    │ UART │  ┌─────────────────────────┐ │
│  │ SceneMgr │ ──────────────┼─────►│  │  uart_bridge.cpp        │ │
│  └──────────┘               │      │  │  lora_agent.cpp         │ │
│  ┌──────────┐  JSON-Events  │ UART │  │  wifi_suite.cpp         │ │
│  │ RX Thread│ ◄─────────────┼─────┤  │  hid_engine.cpp         │ │
│  └──────────┘               │      │  │  payload_store.cpp      │ │
│  ┌──────────┐               │      │  └─────────────────────────┘ │
│  │ json_parse│              │      └──────────────────────────────┘
│  └──────────┘               │
└─────────────────────────────┘
```

**UART-Pinout (Flipper → Heltec):**

| Flipper Pin | Heltec Pin | Signal |
|---|---|---|
| 13 (TX) | RX | UART TX |
| 14 (RX) | TX | UART RX |
| GND | GND | Ground |

---

## UART JSON-Protokoll

### Commands (Flipper → Heltec)

```json
{"cmd":"status"}
{"cmd":"trigger","id":0,"delay_ms":500}
{"cmd":"payload_list"}
{"cmd":"lora_scan"}
{"cmd":"wifi_scan"}
{"cmd":"wifi_deauth","bssid":"XX:XX:XX:XX:XX:XX"}
{"cmd":"evil_portal","ssid":"FreeWiFi"}
{"cmd":"evil_portal_stop"}
{"cmd":"abort"}
```

### Events (Heltec → Flipper)

```json
{"event":"status","state":"idle","lora_rssi":-85,"wifi_clients":0,"bat_pct":87,"fw":"1.0","payload_count":3}
{"event":"payload_list","payloads":[{"id":0,"name":"CH-DE Admin Shell"},{"id":1,"name":"Lock Screen Bypass"}]}
{"event":"lora_scan","results":[...]}
{"event":"log","msg":"WiFi scan started"}
```

---

## Dateistruktur

```
lora_ukfe/
├── application.fam          # FAP-Manifest (appid, name, version, category)
├── lora_ukfe.h              # App-State, Typen, Konstanten
├── lora_ukfe.c              # Entry Point, Log-Buffer, Event-Dispatcher
├── uart_comm.c              # UART-Init, RX-Thread, JSON-Sender
├── json_parse.c             # JSON-Event-Parser (status, payloads, log)
├── images/
│   └── g4meover_icon10.h   # FAP-Icon (10x10 XBM)
└── scenes/
    ├── scenes.h             # Scene-Enum + Handler-Deklarationen
    ├── scene_handlers.c     # on_enter/on_event/on_exit für alle Scenes
    ├── scene_menu.c         # Hauptmenü (10 Einträge)
    ├── scene_status.c       # Live-Status-Anzeige
    ├── scene_mode_menu.c    # Modus-Auswahl
    ├── scene_payload_list.c # Payload-Auswahl + Trigger
    ├── scene_log.c          # JSON-Echtzeit-Log
    └── scene_settings.c     # Einstellungen (Modus, Baud)
```

---

## Build

### Voraussetzungen

- [ufbt](https://github.com/flipperdevices/flipperzero-ufbt) installiert
- G4MEOVER-FW oder kompatible Firmware (API 87.1, mntm-012)

### Kompilieren

```bash
# Im Repo-Verzeichnis:
ufbt

# Ausgabe: build/lora_ukfe.fap (16.984 Bytes)
```

### Auf Flipper deployen

```bash
# Direkt per USB:
ufbt launch

# Oder manuell:
# .fap kopieren nach: SD:/apps/GPIO/lora_ukfe.fap
```

---

## Kompatibilität

| Firmware | Status |
|---|---|
| G4MEOVER-FW v1.0.0 | ✅ Getestet |
| Momentum mntm-012 | ✅ Kompatibel (gleiche API) |
| Unleashed / OFW | ⚠️ Nicht getestet |

---

## Heltec Agent

Der passende Heltec ESP32 LoRa v3 Agent ist Teil des **G4MEOVER UKFE**-Projekts:

- `uart_bridge.cpp` — JSON-Command-Parser
- `lora_agent.cpp` — LoRa TX/RX, Channel-Scan
- `wifi_suite.cpp` — Scan, Deauth, Evil Portal, Beacon Spam
- `hid_engine.cpp` — BadUSB/HID-Payload-Engine
- `payload_store.cpp` — Payload-Verwaltung (SPIFFS)
- `oled_ui.cpp` — OLED-Status-Display

---

## Companion: G4MEOVER-FW

Diese App ist für die **[G4MEOVER-FW](https://github.com/G4MEOVER18/G4MEOVER-FW)** Custom Firmware optimiert, läuft aber auch auf Momentum mntm-012.

| Repo | Inhalt |
|---|---|
| [G4MEOVER-FW](https://github.com/G4MEOVER18/G4MEOVER-FW) | Custom Flipper Zero Firmware (Basis dieser App) |
| [lora-ukfe](https://github.com/G4MEOVER18/lora-ukfe) | Diese App (Flipper FAP) |

---

## Rechtlicher Hinweis

Dieses Tool ist ausschließlich für **autorisierte Sicherheitstests, CTF-Wettbewerbe und Security Research** bestimmt. Die Nutzung gegen Systeme oder Netzwerke ohne ausdrückliche Genehmigung ist illegal. Der Autor übernimmt keine Haftung für Missbrauch.

---

## Support

[![PayPal](https://img.shields.io/badge/PayPal-Spenden-0070ba?style=flat-square&logo=paypal)](https://paypal.me/Freakbank1)
[![Bitcoin](https://img.shields.io/badge/BTC-39vZWmnUwDReQ15BwqQXzyqVQ6U8LardEf-f7931a?style=flat-square&logo=bitcoin)](bitcoin:39vZWmnUwDReQ15BwqQXzyqVQ6U8LardEf)

```
BTC: 39vZWmnUwDReQ15BwqQXzyqVQ6U8LardEf
```

---

## Lizenz

GPL-3.0 — siehe [LICENSE](LICENSE)

---

*Teil der G4MEOVER Security Toolchain · [github.com/G4MEOVER18](https://github.com/G4MEOVER18)*
