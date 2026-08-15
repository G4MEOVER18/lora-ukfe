#include "lora_ukfe.h"
#include "rf/rf_comm.h"
#include <string.h>
#include <stdio.h>
#include <expansion/expansion.h>

// ─── RX-Thread ────────────────────────────────────────────────────────────────

#define RX_LINE_MAX 256

static void uart_rx_callback(FuriHalSerialHandle* handle, FuriHalSerialRxEvent event, void* ctx) {
    UNUSED(handle);
    LораUkfeApp* app = ctx;
    if(event == FuriHalSerialRxEventData) {
        uint8_t byte = furi_hal_serial_async_rx(handle);
        furi_stream_buffer_send(app->rx_stream, &byte, 1, 0);
    }
}

static int32_t rx_thread_fn(void* ctx) {
    LораUkfeApp* app = ctx;
    char line[RX_LINE_MAX];
    size_t pos = 0;

    while(true) {
        uint8_t byte;
        size_t received = furi_stream_buffer_receive(app->rx_stream, &byte, 1, FuriWaitForever);
        if(received == 0) continue;

        if(byte == '\n') {
            line[pos] = '\0';
            if(pos > 1) {
                ukfe_json_handle(app, line);
            }
            pos = 0;
        } else if(byte != '\r' && pos < RX_LINE_MAX - 1) {
            line[pos++] = (char)byte;
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
