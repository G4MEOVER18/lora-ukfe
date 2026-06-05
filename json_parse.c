#include "lora_ukfe.h"
#include <string.h>
#include <stdlib.h>
#include <notification/notification_messages.h>

// Minimaler JSON-String-Extraktor (kein vollständiger Parser — nur für bekannte Felder)

static const char* json_str(const char* json, const char* key, char* out, size_t out_len) {
    char search[64];
    snprintf(search, sizeof(search), "\"%s\":\"", key);
    const char* p = strstr(json, search);
    if(!p) return NULL;
    p += strlen(search);
    size_t i = 0;
    while(*p && *p != '"' && i < out_len - 1) out[i++] = *p++;
    out[i] = '\0';
    return out;
}

static int json_int(const char* json, const char* key) {
    char search[64];
    snprintf(search, sizeof(search), "\"%s\":", key);
    const char* p = strstr(json, search);
    if(!p) return 0;
    p += strlen(search);
    return (int)strtol(p, NULL, 10);
}

// ─── Event-Handler ────────────────────────────────────────────────────────────

static void handle_status(LораUkfeApp* app, const char* json) {
    char tmp[32];
    if(json_str(json, "state", tmp, sizeof(tmp)))
        strlcpy(app->status.state, tmp, sizeof(app->status.state));
    app->status.lora_rssi    = (int16_t)json_int(json, "lora_rssi");
    app->status.wifi_clients = (uint8_t)json_int(json, "wifi_clients");
    app->status.bat_pct      = (uint8_t)json_int(json, "bat_pct");
    if(json_str(json, "fw", tmp, sizeof(tmp)))
        strlcpy(app->status.fw, tmp, sizeof(app->status.fw));

    char line[128];
    snprintf(line, sizeof(line), "[STATUS] %s | RSSI %d dBm | WiFi %d | Bat %d%%",
        app->status.state, app->status.lora_rssi,
        app->status.wifi_clients, app->status.bat_pct);
    ukfe_log_append(app, line);

    view_dispatcher_send_custom_event(app->view_dispatcher, 0x10);
}

static void handle_trigger_ack(LораUkfeApp* app, const char* json) {
    int id = json_int(json, "id");
    bool ok = strstr(json, "\"ok\":true") != NULL;

    char line[64];
    snprintf(line, sizeof(line), "[TRIGGER ACK] ID %d %s", id, ok ? "OK" : "FAIL");
    ukfe_log_append(app, line);

    if(ok) {
        notification_message(app->notifications, &sequence_blink_green_100);
    } else {
        notification_message(app->notifications, &sequence_blink_red_100);
    }
    view_dispatcher_send_custom_event(app->view_dispatcher, 0x11);
}

static void handle_payload_list(LораUkfeApp* app, const char* json) {
    app->payload_count = (uint8_t)json_int(json, "count");

    // Einfacher Payload-Array-Parser: suche {"id":N,"name":"..."}
    app->payload_count = 0;
    const char* p = json;
    while(app->payload_count < UKFE_MAX_PAYLOADS) {
        const char* id_tag = strstr(p, "\"id\":");
        if(!id_tag) break;
        id_tag += 5;
        uint8_t id = (uint8_t)strtol(id_tag, NULL, 10);

        char name[48] = "unknown";
        const char* name_tag = strstr(id_tag, "\"name\":\"");
        if(name_tag) {
            name_tag += 8;
            size_t i = 0;
            while(*name_tag && *name_tag != '"' && i < 47) name[i++] = *name_tag++;
            name[i] = '\0';
        }
        app->payloads[app->payload_count].id = id;
        strlcpy(app->payloads[app->payload_count].name, name, 48);
        app->payload_count++;
        p = name_tag ? name_tag : id_tag + 1;
    }

    char line[64];
    snprintf(line, sizeof(line), "[PAYLOADS] %d gefunden", app->payload_count);
    ukfe_log_append(app, line);
    view_dispatcher_send_custom_event(app->view_dispatcher, 0x12);
}

static void handle_lora_rx(LораUkfeApp* app, const char* json) {
    int rssi = json_int(json, "rssi");
    char data[64] = "";
    json_str(json, "data", data, sizeof(data));

    char line[128];
    snprintf(line, sizeof(line), "[LoRa RX] RSSI %d | %s", rssi, data);
    ukfe_log_append(app, line);
    view_dispatcher_send_custom_event(app->view_dispatcher, 0x13);
}

static void handle_wifi_ap(LораUkfeApp* app, const char* json) {
    char ssid[33] = "";
    json_str(json, "ssid", ssid, sizeof(ssid));
    int rssi = json_int(json, "rssi");
    int ch   = json_int(json, "ch");

    char line[128];
    snprintf(line, sizeof(line), "[WiFi] %-32s RSSI %d CH %d", ssid, rssi, ch);
    ukfe_log_append(app, line);
}

static void handle_error(LораUkfeApp* app, const char* json) {
    char msg[64] = "Unbekannt";
    json_str(json, "msg", msg, sizeof(msg));

    char line[80];
    snprintf(line, sizeof(line), "[ERROR] %s", msg);
    ukfe_log_append(app, line);
    notification_message(app->notifications, &sequence_blink_red_100);
}

// ─── Dispatch ─────────────────────────────────────────────────────────────────

void ukfe_json_handle(LораUkfeApp* app, const char* json) {
    char evt[32] = "";
    json_str(json, "evt", evt, sizeof(evt));

    if     (strcmp(evt, "status")       == 0) handle_status(app, json);
    else if(strcmp(evt, "trigger_ack")  == 0) handle_trigger_ack(app, json);
    else if(strcmp(evt, "payload_list") == 0) handle_payload_list(app, json);
    else if(strcmp(evt, "lora_rx")      == 0) handle_lora_rx(app, json);
    else if(strcmp(evt, "wifi_ap")      == 0) handle_wifi_ap(app, json);
    else if(strcmp(evt, "error")        == 0) handle_error(app, json);
    else {
        // Unbekanntes Event — roh in Log
        ukfe_log_append(app, json);
    }
}
