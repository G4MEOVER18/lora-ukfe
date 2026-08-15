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

// -- WiFi (GhostESP/Marauder/Biscuit-Klasse) --
static const UkfeFn FN_WIFI[] = {
    {"AP-Scan",         UkfeRfCmdWifiScan,    0, false},
    {"Deauth (alle)",   UkfeRfCmdWifiDeauth,  0, false},
    {"Evil Portal",     UkfeRfCmdEvilPortal,  0, false},
    {"Beacon Spam",     UkfeRfCmdBeaconSpam,  0, true},
    {"Handshake",       UkfeRfCmdHandshake,   0, false},
    {"Wardrive + GPS",  UkfeRfCmdWardrive,    0, false},
    {"Probe Sniff",     UkfeRfCmdProbeSniff,  0, false},
    {"Karma",           UkfeRfCmdKarma,       0, false},
    {"Packet Monitor",  UkfeRfCmdPacketMon,   0, false},
    {"Pwnagotchi",      UkfeRfCmdPwnagotchi,  0, false},
    {"Stop",            UkfeRfCmdWifiStop,    0, false},
};

// -- Bluetooth / BLE --
static const UkfeFn FN_BLE[] = {
    {"BLE-Scan",         UkfeRfCmdBleScan,   0, false},
    {"BLE Spam Apple",   UkfeRfCmdBleSpam,   0, true},
    {"BLE Spam Android", UkfeRfCmdBleSpam,   1, true},
    {"BLE Spam Samsung", UkfeRfCmdBleSpam,   2, true},
    {"BLE Spam Windows", UkfeRfCmdBleSpam,   3, true},
    {"Sour Apple",       UkfeRfCmdSourApple, 0, false},
    {"BLE Sniff",        UkfeRfCmdBleSniff,  0, false},
};

// -- SubGHz / LoRa --
static const UkfeFn FN_SUBGHZ[] = {
    {"Status-Ping 868", UkfeRfCmdStatus,      0, false},
    {"LoRa TX",         UkfeRfCmdLoraTx,      0, false},
    {"LoRa Scan",       UkfeRfCmdLoraScan,    0, false},
    {"SubGHz Scan",     UkfeRfCmdSubghzScan,  0, false},
    {"SubGHz Replay",   UkfeRfCmdSubghzReplay,0, false},
    {"RollForge",       UkfeRfCmdRollForge,   0, false},
    {"ProtoPirate",     UkfeRfCmdProtoPirate, 0, false},
    {"Jammer",          UkfeRfCmdJammer,      0, true},
};

// -- GPS --
static const UkfeFn FN_GPS[] = {
    {"GPS-Status",   UkfeRfCmdGpsStatus,   0, false},
    {"GPS-Wardrive", UkfeRfCmdGpsWardrive, 0, false},
};

// -- USB / HID (S3-Satelliten) --
static const UkfeFn FN_HID[] = {
    {"Marker",     UkfeRfCmdHidPayload, 0, true},
    {"Notepad",    UkfeRfCmdHidPayload, 1, true},
    {"PowerShell", UkfeRfCmdHidPayload, 2, true},
    {"CMD Marker", UkfeRfCmdHidPayload, 3, true},
    {"Lock",       UkfeRfCmdHidPayload, 4, true},
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
