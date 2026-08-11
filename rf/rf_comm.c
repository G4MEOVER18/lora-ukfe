// G4MEOVER RF Console — Flipper CC1101-2FSK-Transport (NRZ 9600bps, passend zum Heltec-SX1262-Empfaenger).
#include "rf_comm.h"
#include <furi.h>
#include <furi_hal_power.h>
#include <lib/subghz/devices/devices.h>
#include <lib/toolbox/level_duration.h>
#include <lib/subghz/devices/cc1101_int/cc1101_int_interconnect.h>

#define RF_BIT_US    104u    // 1/9600 s -> 9600 bps NRZ (passend zum SX1262-Empfaenger)
#define RF_PREAMBLE  32u     // 0xAA-Bits zur Bit-Sync
#define RF_SYM_MAX   2048u

// ---- Shared Secret: MUSS mit der Heltec-Seite uebereinstimmen (out-of-band pairen!) ----
static const uint8_t RF_SECRET[UKFE_RF_SECRET_LEN] = {
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,   // "G4MEOVER"
    0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,   // Pairing-Bytes (ersetzen!)
};

typedef struct {
    const SubGhzDevice* device;
    bool     inited;
    uint32_t counter;
    uint8_t  syms[RF_SYM_MAX];  // 0/1-Level je Halbsymbol
    uint16_t sym_count;
    uint16_t sym_pos;
} RfComm;

static RfComm s_rf = {0};

// ---- OOK-TX-Callback: liefert je Halbsymbol eine LevelDuration ----
static LevelDuration rf_tx_cb(void* ctx) {
    RfComm* rf = (RfComm*)ctx;
    if(rf->sym_pos >= rf->sym_count) return level_duration_reset();
    bool lvl = rf->syms[rf->sym_pos++] != 0;
    return level_duration_make(lvl, RF_BIT_US);
}

static void put_sym(RfComm* rf, uint8_t lvl) {
    if(rf->sym_count < RF_SYM_MAX) rf->syms[rf->sym_count++] = lvl ? 1 : 0;
}

// 2-FSK NRZ: ein Bit = ein Symbol (high=f+dev, low=f-dev via 2FSK-Preset).
// Praeambel (0xAA) + Sync 0x4737 + Frame auf UKFE_RF_MAX_FRAME gepaddet
// -> passt zum SX1262-Empfaenger im Fixed-Length-Paketmodus.
static void encode_frame(RfComm* rf, const uint8_t* frame, size_t len) {
    rf->sym_count = 0; rf->sym_pos = 0;
    for(uint32_t i = 0; i < RF_PREAMBLE; i++) put_sym(rf, i & 1);   // 1010... = 0xAA
    for(int i = 15; i >= 0; i--) put_sym(rf, (UKFE_RF_SYNC_WORD >> i) & 1); // Sync MSB first
    for(size_t b = 0; b < UKFE_RF_MAX_FRAME; b++) {
        uint8_t v = (b < len) ? frame[b] : 0x00u;                  // Rest mit 0 padden
        for(int i = 7; i >= 0; i--) put_sym(rf, (v >> i) & 1);     // NRZ, MSB first
    }
    put_sym(rf, 0);  // Idle
}

bool rf_comm_init(void) {
    if(s_rf.inited) return true;
    memset(&s_rf, 0, sizeof(s_rf));
    subghz_devices_init();
    s_rf.device = subghz_devices_get_by_name(SUBGHZ_DEVICE_CC1101_INT_NAME);
    if(!s_rf.device) { subghz_devices_deinit(); return false; }
    subghz_devices_begin(s_rf.device);
    subghz_devices_reset(s_rf.device);
    subghz_devices_load_preset(s_rf.device, FuriHalSubGhzPreset2FSKDev476Async, NULL);
    subghz_devices_set_frequency(s_rf.device, UKFE_RF_FREQUENCY_HZ);
    subghz_devices_idle(s_rf.device);
    s_rf.inited = true;
    return true;
}

void rf_comm_deinit(void) {
    if(!s_rf.inited) return;
    subghz_devices_idle(s_rf.device);
    subghz_devices_sleep(s_rf.device);
    subghz_devices_end(s_rf.device);
    subghz_devices_deinit();
    s_rf.inited = false;
}

bool rf_comm_send(const UkfeRfMessage* in) {
    if(!s_rf.inited || !in) return false;
    UkfeRfMessage msg = *in;
    msg.counter = ++s_rf.counter;   // Rolling-Counter

    uint8_t frame[UKFE_RF_MAX_FRAME];
    size_t n = ukfe_rf_build_frame(RF_SECRET, &msg, frame, sizeof(frame));
    if(n == 0) return false;

    encode_frame(&s_rf, frame, n);

    subghz_devices_idle(s_rf.device);
    if(!subghz_devices_start_async_tx(s_rf.device, rf_tx_cb, &s_rf)) {
        subghz_devices_idle(s_rf.device);
        return false;
    }
    // blockierend auf Abschluss warten (max ~2 s Sicherheitsgrenze)
    uint32_t guard = 0;
    while(!subghz_devices_is_async_complete_tx(s_rf.device) && guard++ < 2000) {
        furi_delay_ms(1);
    }
    subghz_devices_stop_async_tx(s_rf.device);
    subghz_devices_idle(s_rf.device);
    return true;
}

uint32_t rf_comm_counter(void) { return s_rf.counter; }
