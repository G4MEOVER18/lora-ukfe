// G4MEOVER — verschachtelter Befehls-Katalog (Kategorie -> Funktion -> ukfe_rf-Command).
// Sauber, datengetrieben, leicht erweiterbar. Der Heltec (Masterpiece) soll jeden
// Befehl ausfuehren koennen; das Flipper-Menue drillt hier hierarchisch durch.
//
// Geraete-Label im Namen (WER fuehrt aus):
//   [V4+V3] = V4-Hub fuehrt SELBST aus UND spiegelt per ESP-NOW an Satellit V3
//   [V3]    = nur V3-Satellit fuehrt aus (V4 relayt nur; V4 hat keinen Handler)
//   [V4]    = nur V4-Hub
//   [alle]  = jedes Geraet der Kette antwortet
//   [HID]   = HID-faehiges Geraet (S3-Satellit tippt physisch)
//   [Roadmap] = V4-SX1262 vorgesehen, Handler folgt (sendet, noch kein Executor)
// Alles laeuft ueber den V4 (UART vom Flipper) — der faechert zur Flotte auf.
#pragma once
#include "rf/ukfe_rf.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    const char* name;   // Anzeigename im Menue
    uint8_t     cmd;    // ukfe_rf-Command
    uint8_t     arg;    // erster Arg (Mode/Payload-Index); nur wenn has_arg
    bool        has_arg;
} UkfeFn;

typedef struct {
    const char* name;   // Kategorie-Name
    const UkfeFn* fns;
    uint8_t     count;
} UkfeCat;

// -- WiFi (Marauder-Klasse) — V4 fuehrt aus + relayed an V3; Karma nur V3 --
static const UkfeFn FN_WIFI[] = {
    {"AP-Scan [V4+V3]",        UkfeRfCmdWifiScan,   0, false},
    {"Deauth alle [V4+V3]",    UkfeRfCmdWifiDeauth, 0, false},
    {"Evil Twin [V4+V3]",      UkfeRfCmdEvilPortal, 0, false},
    {"Beacon-Spam [V4+V3]",    UkfeRfCmdBeaconSpam, 0, true},
    {"Handshake-Cap [V4+V3]",  UkfeRfCmdHandshake,  0, false},
    {"Probe-Sniff [V4+V3]",    UkfeRfCmdProbeSniff, 0, false},
    {"Packet-Monitor [V4+V3]", UkfeRfCmdPacketMon,  0, false},
    {"Pwnagotchi [V4+V3]",     UkfeRfCmdPwnagotchi, 0, false},
    {"Wardrive+GPS [V4+V3]",   UkfeRfCmdWardrive,   0, false},
    {"Karma [V3]",             UkfeRfCmdKarma,      0, false},
    {"Stop WiFi [V4+V3]",      UkfeRfCmdWifiStop,   0, false},
};

// -- Bluetooth / BLE — V4 fuehrt aus + relayed an V3; Sniff nur V3 --
static const UkfeFn FN_BLE[] = {
    {"BLE-Scan [V4+V3]",       UkfeRfCmdBleScan,   0, false},
    {"Spam Apple [V4+V3]",     UkfeRfCmdBleSpam,   0, true},
    {"Spam Android [V4+V3]",   UkfeRfCmdBleSpam,   1, true},
    {"Spam Samsung [V4+V3]",   UkfeRfCmdBleSpam,   2, true},
    {"Spam Windows [V4+V3]",   UkfeRfCmdBleSpam,   3, true},
    {"Sour Apple [V4+V3]",     UkfeRfCmdSourApple, 0, false},
    {"BLE-Sniff [V3]",         UkfeRfCmdBleSniff,  0, false},
};

// -- Drohnen / Remote-ID — V3-Satellit (ASTM F3411 WiFi-Beacon sniffen) --
static const UkfeFn FN_DRONE[] = {
    {"RemoteID-Scan [V3]",     UkfeRfCmdRidScan,   0, false},
};

// -- LoRaWAN / TTN (Weitverkehr) — 0x60 joint V4 SELBST + spiegelt an V3 (beide -> TTN, SF9) --
static const UkfeFn FN_LORAWAN[] = {
    {"TTN Join+Uplink [V4+V3]", UkfeRfCmdLoraJoin, 0, false},
};

// -- USB / HID (BadUSB) — S3-Satellit tippt physisch --
static const UkfeFn FN_HID[] = {
    {"Marker [HID]",       UkfeRfCmdHidPayload, 0, true},
    {"Notepad [HID]",      UkfeRfCmdHidPayload, 1, true},
    {"PowerShell [HID]",   UkfeRfCmdHidPayload, 2, true},
    {"CMD-Marker [HID]",   UkfeRfCmdHidPayload, 3, true},
    {"Lock [HID]",         UkfeRfCmdHidPayload, 4, true},
    {"DuckyScript [V3]",   UkfeRfCmdHidDucky,   0, true},
    {"HID-Stream [V3]",    UkfeRfCmdHidStream,  0, true},
};

// -- Payloads / Trigger — V3-Satellit (SD-Payload-Bibliothek) --
static const UkfeFn FN_PAYLOAD[] = {
    {"Trigger [V3]",       UkfeRfCmdTrigger,    0, true},
    {"Payload-Run [V3]",   UkfeRfCmdPayloadRun, 0, true},
};

// -- SubGHz / LoRa — V4 (SX1262), Roadmap: sendet, Executor folgt --
static const UkfeFn FN_SUBGHZ[] = {
    {"LoRa-Scan [Roadmap]",   UkfeRfCmdLoraScan,     0, false},
    {"LoRa-TX [Roadmap]",     UkfeRfCmdLoraTx,       0, false},
    {"SubGHz-Scan [Roadmap]", UkfeRfCmdSubghzScan,   0, false},
    {"SubGHz-Replay [Roadmap]",UkfeRfCmdSubghzReplay,0, false},
    {"Jammer [Roadmap]",      UkfeRfCmdJammer,       0, true},
};

// -- System — Steuerung/Status der ganzen Kette --
static const UkfeFn FN_SYS[] = {
    {"Status (alle)", UkfeRfCmdStatus, 0, false},
    {"Ping [V4]",     UkfeRfCmdPing,   0, false},
    {"Abort [V4+V3]", UkfeRfCmdAbort,  0, false},
    {"Reboot Sat.",   UkfeRfCmdReboot, 0, false},
};

#define UKFE_CAT(n, a) {(n), (a), (uint8_t)(sizeof(a) / sizeof((a)[0]))}
static const UkfeCat CATALOG[] = {
    UKFE_CAT("WiFi",         FN_WIFI),
    UKFE_CAT("Bluetooth",    FN_BLE),
    UKFE_CAT("Drohnen/RID",  FN_DRONE),
    UKFE_CAT("LoRaWAN/TTN",  FN_LORAWAN),
    UKFE_CAT("USB-HID",      FN_HID),
    UKFE_CAT("Payloads",     FN_PAYLOAD),
    UKFE_CAT("SubGHz/LoRa",  FN_SUBGHZ),
    UKFE_CAT("System",       FN_SYS),
};
#define CATALOG_COUNT ((int)(sizeof(CATALOG) / sizeof(CATALOG[0])))
