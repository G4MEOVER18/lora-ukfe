// G4MEOVER RF Console — Flipper-Seite: CC1101-OOK-Transport fuer ukfe_rf-Frames.
// Self-contained (statischer Zustand), nutzt subghz_devices wie RollForge.
#pragma once
#include <stdbool.h>
#include "ukfe_rf.h"

// Initialisiert CC1101 (INT), OOK-Preset, 868.35 MHz. false bei Fehler.
bool rf_comm_init(void);
void rf_comm_deinit(void);

// Baut Frame (Secret + interner Rolling-Counter) und sendet ihn per OOK (blockierend).
// true wenn TX komplett angestossen.
bool rf_comm_send(const UkfeRfMessage* msg);

// Baut nur einen ukfe_rf-Frame (Secret + geteilter Rolling-Counter), OHNE Funk-HW —
// fuer den UART/WROOM-Transport. Rueckgabe: Framelaenge in out, 0 bei Fehler.
size_t rf_comm_build_frame(const UkfeRfMessage* msg, uint8_t* out, size_t cap);

// Verifiziert + parst einen empfangenen ukfe_rf-Frame (gemeinsames Secret, MAC+CRC).
// Ohne Replay-Check (Flipper-RX). true = gueltig, out gefuellt.
bool rf_comm_parse_frame(const uint8_t* in, size_t len, UkfeRfMessage* out);

// Aktuellen Counter-Stand lesen (fuer Anzeige/Persistenz).
uint32_t rf_comm_counter(void);
