# G4MEOVER RF Console — 868-FSK Command Protocol v1

Funk-Steuerkanal zwischen **Flipper (CC1101)** und **Heltec LoRa v3 (SX1262)** — ersetzt die bisherige UART/JSON-Bridge. Beide Seiten teilen `ukfe_rf.h`.

## 1. Physical Layer (beide Chips beherrschen es)
| Parameter | Wert | Grund |
|---|---|---|
| Frequenz | **868.350 MHz** | EU-ISM, CC1101 + SX1262 FSK identisch |
| Modulation | **2-FSK** | CC1101 kann kein LoRa-Chirp → gemeinsamer Nenner |
| Bitrate | 9.6 kbps | robust, beide Chips stabil |
| Deviation | 25 kHz | Standard für 9.6k FSK |
| Preamble | ≥ 4 B | Sync-Erkennung |
| Sync-Word | `0x4737` ("G7") | Paketfilter (CC1101 packet mode) |
| Encoding | Whitening an, CRC on chip | zusätzlich zu App-CRC |

## 2. Frame (nach Sync-Word, max 61 B Payload bei CC1101 FIFO)
```
[LEN 1B] [MAGIC 1B=0x47] [VER 1B=0x01] [COUNTER 4B LE] [CMD 1B] [ALEN 1B] [ARGS ALEN B] [MAC 4B] [CRC16 2B]
```
- **COUNTER**: monoton steigend, 32 bit. Anti-Replay.
- **MAC**: 4 B = erste 4 Bytes von `HMAC-lite(secret, VER|COUNTER|CMD|ALEN|ARGS)` (siehe §5). Verhindert Fälschung *und* koppelt den Counter kryptografisch.
- **CRC16**: CCITT über LEN..MAC (Übertragungsfehler).

## 3. Commands (Flipper → Heltec)
| Cmd | Wert | Args | entspricht altem JSON |
|---|---|---|---|
| STATUS | 0x01 | – | `{"cmd":"status"}` |
| TRIGGER | 0x02 | id 1B, delay_ms 4B LE | `{"cmd":"trigger",...}` |
| ABORT | 0x03 | – | `{"cmd":"abort"}` |
| PAYLOAD_LIST | 0x04 | – | `{"cmd":"payload_list"}` |
| PAYLOAD_RUN | 0x05 | idx 1B | – |
| LORA_SCAN | 0x10 | start 4B, end 4B, dwell 2B (kHz/ms) | `{"cmd":"lora_scan",...}` |
| WIFI_SCAN | 0x20 | – | `{"cmd":"wifi_scan"}` |
| WIFI_DEAUTH | 0x21 | bssid 6B, ch 1B | `{"cmd":"wifi_deauth"}` |
| WIFI_STOP | 0x22 | – | deauth_stop |
| EVIL_PORTAL | 0x23 | portal_id 1B | `{"cmd":"evil_portal"}` |
| BEACON_SPAM | 0x24 | mode 1B | `{"cmd":"wifi_beacon_spam"}` |

## 4. Responses (Heltec → Flipper) — CMD mit gesetztem Bit 7 (0x80|cmd)
| Resp | Wert | Payload |
|---|---|---|
| ACK | 0x80 | orig_cmd 1B, result 1B (0=ok) |
| STATUS_RESP | 0x81 | mode 1B, busy 1B, batt 1B, rssi 1B |
| PAYLOAD_ITEM | 0x84 | idx 1B, name-string (ALEN) |
| SCAN_HIT | 0x90 | freq/ch 4B, rssi 1B |
Responses tragen denselben COUNTER wie der auslösende Befehl (Zuordnung).

## 5. Anti-Replay + Auth (Rolling-Counter, wie bei KeeLoq — auf eigenem Kanal)
- Shared Secret (16 B) in beiden Firmwares (out-of-band gepairt, nicht im Repo).
- **Heltec** akzeptiert einen Frame nur, wenn: MAC stimmt **und** `COUNTER > last_accepted_counter` (Fenster +1000 gegen Desync, sonst Re-Pair).
- Nach Annahme: `last_accepted_counter = COUNTER`.
- Abgefangene/wiederholte Frames sind wertlos (Counter verbraucht) — verhindert Replay eines mitgeschnittenen "trigger"/"deauth".
- MAC = truncated keyed hash; ohne Secret nicht fälschbar → auch neue Counter nicht vorspielbar.

## 6. Fallback
UART/JSON bleibt als Fallback erhalten (`UkfeModeDirect`); RF ist der neue Default für `UkfeModeSubGhz`/drahtlos.

## 7. Implementierung
- **Flipper:** `ukfe_rf.c` nutzt `subghz_devices_*` (CC1101 int/ext) im FSK-Paketmodus. TX: `ukfe_rf_build_frame()` → senden. RX: Sync-Filter → `ukfe_rf_parse_frame()`.
- **Heltec:** SX1262 im (G)FSK-Paketmodus mit identischem Sync/Whitening; RX-Dispatcher ersetzt `uart_bridge`-JSON-Parser, ruft dieselben Handler.
