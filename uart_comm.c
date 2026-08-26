#include "lora_ukfe.h"
#include "rf/rf_comm.h"
#include <string.h>
#include <stdio.h>
#include <expansion/expansion.h>

// ─── RX-Thread ────────────────────────────────────────────────────────────────

static void uart_rx_callback(FuriHalSerialHandle* handle, FuriHalSerialRxEvent event, void* ctx) {
    UNUSED(handle);
    LораUkfeApp* app = ctx;
    if(event == FuriHalSerialRxEventData) {
        uint8_t byte = furi_hal_serial_async_rx(handle);
        furi_stream_buffer_send(app->rx_stream, &byte, 1, 0);
    }
}

// Verarbeitet einen validierten ukfe_rf-Response-Frame vom Hub/Satelliten.
static void handle_rf_response(LораUkfeApp* app, const UkfeRfMessage* m) {
    char line[96];
    switch(m->cmd) {
    case UkfeRfRespAck:
        snprintf(line, sizeof(line), "[ACK] cmd=0x%02X res=%u",
            m->arg_len > 0 ? m->args[0] : 0, m->arg_len > 1 ? m->args[1] : 0);
        notification_message(app->notifications, &sequence_blink_green_100);
        break;
    case UkfeRfRespStatus: {
        uint8_t mode = m->arg_len > 0 ? m->args[0] : 0;
        uint8_t busy = m->arg_len > 1 ? m->args[1] : 0;
        uint8_t batt = m->arg_len > 2 ? m->args[2] : 0;
        int8_t  rssi = (int8_t)(m->arg_len > 3 ? m->args[3] : 0);
        app->status.bat_pct = batt;
        app->status.lora_rssi = rssi;
        strlcpy(app->status.state, busy ? "running" : "idle", sizeof(app->status.state));
        snprintf(line, sizeof(line), "[STATUS] %s bat=%u%% rssi=%d mode=%u",
            busy ? "running" : "idle", batt, rssi, mode);
        view_dispatcher_send_custom_event(app->view_dispatcher, 0x10);
        break;
    }
    case UkfeRfRespRelayed:
        snprintf(line, sizeof(line), "[RELAY] cmd=0x%02X ok=%u",
            m->arg_len > 0 ? m->args[0] : 0, m->arg_len > 1 ? m->args[1] : 0);
        break;
    default:
        snprintf(line, sizeof(line), "[RX] cmd=0x%02X ctr=%lu alen=%u",
            m->cmd, (unsigned long)m->counter, m->arg_len);
        break;
    }
    ukfe_log_append(app, line);
}

// Byte-Stream-Frame-Scanner (identisch zum V4-Hub): sucht LEN|MAGIC|VER, wartet auf
// den Rest, verifiziert (MAC+CRC) und dispatcht. Ersetzt den alten JSON-Zeilenparser
// -> Flipper und Hub sprechen jetzt auf beiden Richtungen dasselbe Frame-Format.
static int32_t rx_thread_fn(void* ctx) {
    LораUkfeApp* app = ctx;
    static uint8_t buf[UKFE_RF_MAX_FRAME * 2];
    size_t len = 0;

    while(true) {
        uint8_t byte;
        size_t received = furi_stream_buffer_receive(app->rx_stream, &byte, 1, FuriWaitForever);
        if(received == 0) continue;
        if(len < sizeof(buf)) buf[len++] = byte;

        // Solange ein vollstaendiger Frame im Puffer liegt, verarbeiten.
        while(len >= UKFE_RF_HDR_OVERHEAD) {
            size_t i = 0;
            bool found = false;
            for(; i + 2 < len; i++) {
                size_t rl = (size_t)buf[i] + 1;
                if(rl < UKFE_RF_HDR_OVERHEAD || rl > UKFE_RF_MAX_FRAME) continue;
                if(buf[i + 1] != UKFE_RF_MAGIC || buf[i + 2] != UKFE_RF_VERSION) continue;
                found = true;
                break;
            }
            if(!found) {
                // Kein Frame-Start -> nur die letzten 2 Bytes als moeglichen Anfang behalten.
                if(len > 2) {
                    memmove(buf, buf + (len - 2), 2);
                    len = 2;
                }
                break;
            }
            if(i > 0) {
                memmove(buf, buf + i, len - i);
                len -= i;
            }
            size_t rl = (size_t)buf[0] + 1;
            if(len < rl) break; // Rest des Frames noch nicht da
            UkfeRfMessage msg;
            if(rf_comm_parse_frame(buf, rl, &msg)) {
                handle_rf_response(app, &msg);
            }
            memmove(buf, buf + rl, len - rl);
            len -= rl;
        }
    }
    return 0;
}

// ─── Init / Deinit ────────────────────────────────────────────────────────────

bool ukfe_uart_init(LораUkfeApp* app) {
    app->rx_stream = furi_stream_buffer_alloc(UKFE_RX_BUF_SIZE, 1);
    if(!app->rx_stream) return false;

    // Expansion-Dienst abschalten — er belegt sonst die USART (Pin 13/14), dann
    // scheitert der Acquire und "NET:" sendet still nichts (haeufigste Ursache).
    Expansion* exp = furi_record_open(RECORD_EXPANSION);
    expansion_disable(exp);
    furi_record_close(RECORD_EXPANSION);

    app->serial = furi_hal_serial_control_acquire(UKFE_UART_ID);
    if(!app->serial) {
        furi_stream_buffer_free(app->rx_stream);
        app->rx_stream = NULL;
        return false;
    }

    furi_hal_serial_init(app->serial, UKFE_UART_BAUD);
    furi_hal_serial_async_rx_start(app->serial, uart_rx_callback, app, false);

    app->rx_thread = furi_thread_alloc_ex("UkfeRx", 1024, rx_thread_fn, app);
    furi_thread_start(app->rx_thread);

    return true;
}

void ukfe_uart_deinit(LораUkfeApp* app) {
    if(app->rx_thread) {
        furi_thread_join(app->rx_thread);
        furi_thread_free(app->rx_thread);
        app->rx_thread = NULL;
    }
    if(app->serial) {
        furi_hal_serial_async_rx_stop(app->serial);
        furi_hal_serial_deinit(app->serial);
        furi_hal_serial_control_release(app->serial);
        app->serial = NULL;
    }
    if(app->rx_stream) {
        furi_stream_buffer_free(app->rx_stream);
        app->rx_stream = NULL;
    }

    // Expansion-Dienst wieder aktivieren (Normalzustand des Flippers).
    Expansion* exp = furi_record_open(RECORD_EXPANSION);
    expansion_enable(exp);
    furi_record_close(RECORD_EXPANSION);
}

// ─── Senden ───────────────────────────────────────────────────────────────────

void ukfe_uart_send_cmd(LораUkfeApp* app, const char* json) {
    if(!app->serial) return;
    size_t len = strlen(json);
    furi_hal_serial_tx(app->serial, (const uint8_t*)json, len);
    // CR/LF Terminator
    furi_hal_serial_tx(app->serial, (const uint8_t*)"\r\n", 2);
    ukfe_log_append(app, json);
}

void ukfe_uart_send_status(LораUkfeApp* app) {
    ukfe_uart_send_cmd(app, "{\"cmd\":\"status\"}");
}

void ukfe_uart_send_trigger(LораUkfeApp* app, uint8_t id, uint32_t delay_ms) {
    char buf[64];
    if(delay_ms > 0) {
        snprintf(buf, sizeof(buf),
            "{\"cmd\":\"trigger\",\"id\":%u,\"delay_ms\":%lu}", id, delay_ms);
    } else {
        snprintf(buf, sizeof(buf), "{\"cmd\":\"trigger\",\"id\":%u}", id);
    }
    ukfe_uart_send_cmd(app, buf);
}

void ukfe_uart_send_payload_list(LораUkfeApp* app) {
    ukfe_uart_send_cmd(app, "{\"cmd\":\"payload_list\"}");
}

void ukfe_uart_send_abort(LораUkfeApp* app) {
    ukfe_uart_send_cmd(app, "{\"cmd\":\"abort\"}");
}

void ukfe_uart_send_lora_scan(LораUkfeApp* app) {
    ukfe_uart_send_cmd(app,
        "{\"cmd\":\"lora_scan\",\"start\":863000,\"end\":870000,\"dwell_ms\":50}");
}

void ukfe_uart_send_wifi_scan(LораUkfeApp* app) {
    ukfe_uart_send_cmd(app, "{\"cmd\":\"wifi_scan\"}");
}

// Sendet einen ukfe_rf-Binaerframe ueber die GPIO-UART (Pin 13/14) an den
// WROOM-Relay, der ihn per ESP-NOW an die Satelliten weiterreicht. Gleiches
// Frame-Format wie der 868-Funk -> Satelliten validieren beide Transporte gleich.
void ukfe_uart_send_rf(LораUkfeApp* app, const UkfeRfMessage* m) {
    if(!app->serial || !m) return;
    uint8_t frame[UKFE_RF_MAX_FRAME];
    size_t n = rf_comm_build_frame(m, frame, sizeof(frame));
    if(n == 0) return;
    furi_hal_serial_tx(app->serial, frame, n);
    char note[40];
    snprintf(note, sizeof(note), "[UART->WROOM] cmd=0x%02X (%u B)", m->cmd, (unsigned)n);
    ukfe_log_append(app, note);
}
