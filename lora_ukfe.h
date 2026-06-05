#pragma once

#include <furi.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/submenu.h>
#include <gui/modules/text_box.h>
#include <gui/modules/popup.h>
#include <gui/modules/variable_item_list.h>
#include <furi_hal_serial.h>
#include <furi_hal_serial_control.h>
#include <notification/notification_messages.h>

// ─── Protokoll ────────────────────────────────────────────────────────────────

#define UKFE_UART_BAUD     115200
#define UKFE_UART_ID       FuriHalSerialIdUsart
#define UKFE_JSON_BUF_SIZE 512
#define UKFE_RX_BUF_SIZE   1024

// ─── Kommunikations-Modi ──────────────────────────────────────────────────────

typedef enum {
    UkfeModeLora    = 0,
    UkfeModeWifi    = 1,
    UkfeModeSubGhz  = 2,
    UkfeModeDirect  = 3,
    UkfeModeCount,
} UkfeMode;

// ─── Heltec-Status (geparst aus JSON-Events) ──────────────────────────────────

typedef struct {
    char    state[16];        // idle / running / wait / error
    int16_t lora_rssi;
    uint8_t wifi_clients;
    uint8_t bat_pct;
    char    fw[16];
    uint8_t payload_count;
} UkfeHeltecStatus;

typedef struct {
    uint8_t id;
    char    name[48];
} UkfePayloadEntry;

#define UKFE_MAX_PAYLOADS 32

// ─── App-State ────────────────────────────────────────────────────────────────

typedef struct {
    // GUI
    ViewDispatcher* view_dispatcher;
    SceneManager*   scene_manager;
    Submenu*        submenu;
    TextBox*        text_box;       // UkfeViewStatus
    TextBox*        log_box;        // UkfeViewLog (separates Widget!)
    Popup*          popup;
    VariableItemList* var_list;

    // UART
    FuriHalSerialHandle* serial;
    FuriThread*          rx_thread;
    FuriStreamBuffer*    rx_stream;
    uint8_t              rx_buf[UKFE_RX_BUF_SIZE];

    // Status
    UkfeMode            mode;
    UkfeHeltecStatus    status;
    UkfePayloadEntry    payloads[UKFE_MAX_PAYLOADS];
    uint8_t             payload_count;

    // Anzeige-Buffer
    char                log_buf[UKFE_JSON_BUF_SIZE * 4];
    FuriMutex*          log_mutex;

    // Notifications
    NotificationApp*    notifications;
} LораUkfeApp;

// ─── Views ────────────────────────────────────────────────────────────────────

typedef enum {
    UkfeViewMenu = 0,
    UkfeViewStatus,
    UkfeViewPayloadList,
    UkfeViewLog,
    UkfeViewSettings,
} UkfeView;

// ─── Funktionen ───────────────────────────────────────────────────────────────

// uart_comm.c
bool ukfe_uart_init(LораUkfeApp* app);
void ukfe_uart_deinit(LораUkfeApp* app);
void ukfe_uart_send_cmd(LораUkfeApp* app, const char* json);
void ukfe_uart_send_status(LораUkfeApp* app);
void ukfe_uart_send_trigger(LораUkfeApp* app, uint8_t id, uint32_t delay_ms);
void ukfe_uart_send_payload_list(LораUkfeApp* app);
void ukfe_uart_send_abort(LораUkfeApp* app);
void ukfe_uart_send_lora_scan(LораUkfeApp* app);
void ukfe_uart_send_wifi_scan(LораUkfeApp* app);

// json_parse.c
void ukfe_json_handle(LораUkfeApp* app, const char* json);

// lora_ukfe.c
int32_t lora_ukfe_app(void* p);
void    ukfe_log_append(LораUkfeApp* app, const char* line);
