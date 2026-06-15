# lora_ukfe — USB Army Penetrator Flipper Edition

> Flipper Zero FAP · Remote-Steuerung für den G4MEOVER Agent via UART/JSON

![Version](https://img.shields.io/badge/version-1.0.0-blue)
![API](https://img.shields.io/badge/Flipper%20API-87.1-orange)
![SDK](https://img.shields.io/badge/SDK-mntm--012-purple)
![Category](https://img.shields.io/badge/category-GPIO-green)
![License](https://img.shields.io/badge/license-GPL--3.0-red)

---

```
 ██████╗ ██╗  ██╗███╗   ███╗███████╗ ██████╗ ██╗   ██╗███████╗██████╗
██╔════╝ ██║  ██║████╗ ████║██╔════╝██╔═══██╗██║   ██║██╔════╝██╔══██╗
██║  ███╗███████║██╔████╔██║█████╗  ██║   ██║██║   ██║█████╗  ██████╔╝
██║   ██║╚════██║██║╚██╔╝██║██╔══╝  ██║   ██║╚██╗ ██╔╝██╔══╝  ██╔══██╗
╚██████╔╝     ██║██║ ╚═╝ ██║███████╗╚██████╔╝ ╚████╔╝ ███████╗██║  ██║
 ╚═════╝      ╚═╝╚═╝     ╚═╝╚══════╝ ╚═════╝   ╚═══╝  ╚══════╝╚═╝  ╚═╝
                  USB Army Penetrator Flipper Edition
```

---

## Übersicht

`lora_ukfe` ist eine Flipper Zero App (`.fap`), die als Fernbedienung für den **USB Army Penetrator** Agenten dient. Kommunikation über UART (115200 Baud) mit JSON-Protokoll.

### Unterstützte Hardware (Agent-Seite)

| Gerät | Firmware | Status |
|---|---|---|
| **Heltec ESP32 LoRa v3** | [G4MEOVER LoRa Agent v1.0](https://github.com/G4MEOVER18/lora-ukfe/releases) | ✅ Vollständig |
| **LilyGo T-Dongle S3** | G4MEOVER Agent (in Vorbereitung) | 🔧 Bald |
| **ESP32 CYD** | G4MEOVER Agent (in Vorbereitung) | 🔧 Bald |

> **Nur G4MEOVER-Firmware wird unterstützt.** Original-Firmware, radioFritz oder andere Drittanbieter-Firmware sind nicht kompatibel.

### Flipper-Firmware-Kompatibilität

| Firmware | Status |
|---|---|
| G4MEOVER-FW v1.0.0 | ✅ Getestet |
| Momentum mntm-012 | ✅ Kompatibel |

### Features

| Funktion | Beschreibung |
|---|---|
| **Status** | Live-Status vom Agent (State, RSSI, WiFi-Clients, Battery) |
| **Trigger** | BadUSB/HID-Payloads remote auslösen (mit optionalem Delay) |
| **Payload-Liste** | Alle auf dem Agent gespeicherten Payloads anzeigen |
| **LoRa Scan** | Umgebungs-Scan auf 868 MHz EU-Band |
| **WiFi Scan** | SSID-Scan über ESP32 WiFi |
| **WiFi Deauth** | Deauth-Frame-Sender (nur für autorisierte Pentests) |
| **Evil Portal** | Captive Portal starten/stoppen |
| **ABORT** | Sofortabbruch aller laufenden Aktionen |
| **Log** | JSON-Echtzeit-Log der Agent-Kommunikation |
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

**UART-Pinout (Flipper → Heltec ESP32 LoRa v3):**

| Flipper Pin | Heltec Pin | Signal |
|---|---|---|
| 13 (TX) | RX | UART TX |
| 14 (RX) | TX | UART RX |
| GND | GND | Ground |

---

## UART JSON-Protokoll

### Commands (Flipper → Agent)

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

### Events (Agent → Flipper)

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
├── application.fam          # FAP-Manifest
├── lora_ukfe.h              # App-State, Typen, Konstanten
├── lora_ukfe.c              # Entry Point, Log-Buffer, Event-Dispatcher
├── uart_comm.c              # UART-Init, RX-Thread, JSON-Sender
├── json_parse.c             # JSON-Event-Parser
├── images/
│   └── g4meover_icon10.h   # FAP-Icon (10x10 XBM)
└── scenes/
    ├── scene_menu.c         # Hauptmenü (10 Einträge)
    ├── scene_status.c       # Live-Status-Anzeige
    ├── scene_payload_list.c # Payload-Auswahl + Trigger
    ├── scene_log.c          # JSON-Echtzeit-Log
    └── scene_settings.c     # Einstellungen
```

---

## Build

```bash
# Im Repo-Verzeichnis (API 87.1 / mntm-012 SDK):
ufbt

# Direkt deployen:
ufbt launch
```

---

## Agent-Firmware (Heltec ESP32 LoRa v3)

Die G4MEOVER Firmware für den Heltec ESP32 LoRa v3 ist Teil dieses Repos:

- **Releases:** [github.com/G4MEOVER18/lora-ukfe/releases](https://github.com/G4MEOVER18/lora-ukfe/releases)
- Datei: `G4MEOVER-LoRa-Agent-v1.0.bin`
- Flashen: `esptool.py --chip esp32s3 write_flash 0x0 G4MEOVER-LoRa-Agent-v1.0.bin`

**Module des Agents:**

| Modul | Funktion |
|---|---|
| `uart_bridge.cpp` | JSON-Command-Parser |
| `lora_agent.cpp` | LoRa TX/RX, Channel-Scan |
| `wifi_suite.cpp` | Scan, Deauth, Evil Portal, Beacon Spam |
| `hid_engine.cpp` | BadUSB/HID-Payload-Engine |
| `payload_store.cpp` | Payload-Verwaltung (SPIFFS) |
| `oled_ui.cpp` | OLED-Status-Display |

---

## Verwandte Projekte

| Projekt | Beschreibung |
|---|---|
| [G4MEOVER-FW](https://github.com/G4MEOVER18/G4MEOVER-FW) | Custom Flipper Zero Firmware |
| [ProtoPirate](https://github.com/G4MEOVER18/ProtoPirate) | Car Keyfob RF Decoder/Emulator (27+ Protokolle) |
| [RollJam](https://github.com/G4MEOVER18/RollJam) | RollJam Attack PoC (Jam + Capture + Replay) |
| [RollLab](https://github.com/G4MEOVER18/RollLab) | Rolling Code Vulnerability Lab |

---

## Rechtlicher Hinweis

Dieses Tool ist ausschließlich für **autorisierte Sicherheitstests, CTF-Wettbewerbe und Security Research** an eigener Hardware bestimmt. Die Nutzung gegen Systeme ohne Genehmigung ist illegal.

---

## Support

[![PayPal](https://img.shields.io/badge/PayPal-Spenden-0070ba?style=flat-square&logo=paypal)](https://paypal.me/Freakbank1)
[![Bitcoin](https://img.shields.io/badge/BTC-39vZWmnUwDReQ15BwqQXzyqVQ6U8LardEf-f7931a?style=flat-square&logo=bitcoin)](bitcoin:39vZWmnUwDReQ15BwqQXzyqVQ6U8LardEf)

```
BTC: 39vZWmnUwDReQ15BwqQXzyqVQ6U8LardEf

**Kontakt:** [g4me.over.18@gmail.com](mailto:g4me.over.18@gmail.com)
```

---

## Lizenz

GPL-3.0 — siehe [LICENSE](LICENSE)

---

*Teil der G4MEOVER Security Toolchain · [github.com/G4MEOVER18](https://github.com/G4MEOVER18)*
