#include "../lora_ukfe.h"
#include "scenes.h"
#include <gui/modules/text_box.h>

static void refresh_display(LораUkfeApp* app) {
    char buf[256];
    snprintf(buf, sizeof(buf),
        "Modus:  %s\n"
        "Status: %s\n"
        "RSSI:   %d dBm\n"
        "WiFi:   %d Clients\n"
        "Bat:    %d%%\n"
        "FW:     %s\n"
        "Paylds: %d",
        (const char*[]){"LoRa (Heltec)","WiFi (T-Dongle)","SubGHz OOK","Direkt"}[app->mode],
        app->status.state,
        app->status.lora_rssi,
        app->status.wifi_clients,
        app->status.bat_pct,
        app->status.fw[0] ? app->status.fw : "--",
        app->payload_count
    );
    text_box_set_text(app->text_box, buf);
}

void scene_status_on_enter(void* ctx) {
    LораUkfeApp* app = ctx;
    text_box_reset(app->text_box);
    text_box_set_font(app->text_box, TextBoxFontText);
    refresh_display(app);
    view_dispatcher_switch_to_view(app->view_dispatcher, UkfeViewStatus);
    ukfe_uart_send_status(app);
}

bool scene_status_on_event(void* ctx, SceneManagerEvent event) {
    LораUkfeApp* app = ctx;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == 0x10) {
            // Status-Update empfangen
            refresh_display(app);
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        scene_manager_previous_scene(app->scene_manager);
        consumed = true;
    } else if(event.type == SceneManagerEventTypeTick) {
        // Alle 3 Sekunden Status abfragen
        static uint32_t tick_count = 0;
        if(++tick_count >= 3000 / 100) {
            tick_count = 0;
            ukfe_uart_send_status(app);
        }
    }
    return consumed;
}

void scene_status_on_exit(void* ctx) {
    LораUkfeApp* app = ctx;
    text_box_reset(app->text_box);
}
