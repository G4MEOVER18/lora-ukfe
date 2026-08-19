// G4MEOVER — verschachtelter Befehls-Katalog (Kategorie -> Funktion -> ukfe_rf-Command).
// Sauber, datengetrieben, leicht erweiterbar. Der Heltec (Masterpiece) soll jeden
// Befehl ausfuehren koennen; das Flipper-Menue drillt hier hierarchisch durch.
// (Naechste Stufe: pro Funktion Geraet/Funktechnik/Alternative waehlbar.)
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

// Geraete-Label im Namen: [V4]=Basis-Executor, [V4+V3]=V4 fuehrt aus + Satellit V3,
// [V4 GPS]=braucht V4-GPS, [HID]=HID-Geraet (V4/V3). Alles laeuft ueber V4 (UART).

// -- WiFi (Marauder-Klasse) — V4 fuehrt aus, relayed an V3 --
static const UkfeFn FN_WIFI[] = {
    {"AP-Scan [V4+V3]",       UkfeRfCmdWifiScan,    0, false},
    {"Deauth alle [V4+V3]",   UkfeRfCmdWifiDeauth,  0, false},
    {"Evil Twin [V4+V3]",     UkfeRfCmdEvilPortal,  0, false},
    {"Beacon Spam [V4+V3]",   UkfeRfCmdBeaconSpam,  0, true},
    {"Handshake [V4+V3]",     UkfeRfCmdHandshake,   0, false},
    {"Wardrive+GPS [V4]",     UkfeRfCmdWardrive,    0, false},
    {"Probe Sniff [V4+V3]",   UkfeRfCmdProbeSniff,  0, false},
    {"Karma [V4+V3]",         UkfeRfCmdKarma,       0, false},
    {"Packet Mon [V4+V3]",    UkfeRfCmdPacketMon,   0, false},
    {"Pwnagotchi [V4+V3]",    UkfeRfCmdPwnagotchi,  0, false},
    {"Stop [V4+V3]",          UkfeRfCmdWifiStop,    0, false},
};

// -- Bluetooth / BLE — V4 fuehrt aus, relayed an V3 --
static const UkfeFn FN_BLE[] = {
    {"BLE-Scan [V4+V3]",       UkfeRfCmdBleScan,   0, false},
    {"BLE Spam Apple [V4+V3]", UkfeRfCmdBleSpam,   0, true},
    {"BLE Spam Android [V4+V3]",UkfeRfCmdBleSpam,  1, true},
    {"BLE Spam Samsung [V4+V3]",UkfeRfCmdBleSpam,  2, true},
    {"BLE Spam Win [V4+V3]",   UkfeRfCmdBleSpam,   3, true},
    {"Sour Apple [V4+V3]",     UkfeRfCmdSourApple, 0, false},
    {"BLE Sniff [V4+V3]",      UkfeRfCmdBleSniff,  0, false},
};

// -- SubGHz / LoRa — V4 (SX1262) --
static const UkfeFn FN_SUBGHZ[] = {
    {"Status-Ping [alle]",  UkfeRfCmdStatus,      0, false},
    {"LoRa TX [V4]",        UkfeRfCmdLoraTx,      0, false},
    {"LoRa Scan [V4]",      UkfeRfCmdLoraScan,    0, false},
    {"SubGHz Scan [V4]",    UkfeRfCmdSubghzScan,  0, false},
    {"SubGHz Replay [V4]",  UkfeRfCmdSubghzReplay,0, false},
    {"RollForge [V4]",      UkfeRfCmdRollForge,   0, false},
    {"ProtoPirate [V4]",    UkfeRfCmdProtoPirate, 0, false},
    {"Jammer [V4]",         UkfeRfCmdJammer,      0, true},
};

// -- GPS — V4 (primaer + sekundaer) --
static const UkfeFn FN_GPS[] = {
    {"GPS-Status [V4]",   UkfeRfCmdGpsStatus,   0, false},
    {"GPS-Wardrive [V4]", UkfeRfCmdGpsWardrive, 0, false},
};

// -- USB / HID (BadUSB) — HID-Geraet (V4/V3) --
static const UkfeFn FN_HID[] = {
    {"Marker [HID]",     UkfeRfCmdHidPayload, 0, true},
    {"Notepad [HID]",    UkfeRfCmdHidPayload, 1, true},
    {"PowerShell [HID]", UkfeRfCmdHidPayload, 2, true},
    {"CMD Marker [HID]", UkfeRfCmdHidPayload, 3, true},
    {"Lock [HID]",       UkfeRfCmdHidPayload, 4, true},
};

// -- System --
static const UkfeFn FN_SYS[] = {
    {"Status (alle)", UkfeRfCmdStatus, 0, false},
    {"Ping",          UkfeRfCmdPing,   0, false},
    {"Abort",         UkfeRfCmdAbort,  0, false},
    {"Reboot Sat.",   UkfeRfCmdReboot, 0, false},
};

#define UKFE_CAT(n, a) {(n), (a), (uint8_t)(sizeof(a) / sizeof((a)[0]))}
static const UkfeCat CATALOG[] = {
    UKFE_CAT("WiFi",        FN_WIFI),
    UKFE_CAT("Bluetooth",   FN_BLE),
    UKFE_CAT("SubGHz/LoRa", FN_SUBGHZ),
    UKFE_CAT("GPS",         FN_GPS),
    UKFE_CAT("USB-HID",     FN_HID),
    UKFE_CAT("System",      FN_SYS),
};
#define CATALOG_COUNT ((int)(sizeof(CATALOG) / sizeof(CATALOG[0])))
