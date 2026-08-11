#include "../lora_ukfe.h"
#include "scenes.h"
#include "../rf/rf_comm.h"

typedef enum {
    MenuItemStatus = 0,
    MenuItemTrigger,
    MenuItemPayloads,
    MenuItemLoRaScan,
    MenuItemWiFiScan,
    MenuItemRfPing,
    MenuItemRfHid0,
    MenuItemRfHid1,
    MenuItemLog,
    MenuItemSettings,
} MenuItems;

static void menu_callback(void* ctx, uint32_t index) {
    LораUkfeApp* app = ctx;
    switch((MenuItems)index) {
    case MenuItemStatus:
        scene_manager_next_scene(app->scene_manager, UkfeSceneStatus);
        break;
    case MenuItemTrigger:
        scene_manager_next_scene(app->scene_manager, UkfeScenePayloadList);
        break;
    case MenuItemPayloads:
        scene_manager_next_scene(app->scene_manager, UkfeScenePayloadList);
        break;
    case MenuItemLoRaScan:
        ukfe_uart_send_lora_scan(app);
        scene_manager_next_scene(app->scene_manager, UkfeSceneLog);
        break;
    case MenuItemWiFiScan:
        ukfe_uart_send_wifi_scan(app);
        scene_manager_next_scene(app->scene_manager, UkfeSceneLog);
        break;
    case MenuItemRfPing: {
        UkfeRfMessage m;
        ukfe_rf_make_simple(&m, UkfeRfCmdStatus);
        if(rf_comm_init()) rf_comm_send(&m);
        scene_manager_next_scene(app->scene_manager, UkfeSceneLog);
        break;
    }
    case MenuItemRfHid0: {
        UkfeRfMessage m;
        ukfe_rf_make_trigger(&m, 0, 0);   // id=0 -> Heltec hid_payload(0): Marker-Test
        if(rf_comm_init()) rf_comm_send(&m);
        scene_manager_next_scene(app->scene_manager, UkfeSceneLog);
        break;
    }
    case MenuItemRfHid1: {
        UkfeRfMessage m;
        ukfe_rf_make_trigger(&m, 1, 0);   // id=1 -> Heltec hid_payload(1): Win+R
        if(rf_comm_init()) rf_comm_send(&m);
        scene_manager_next_scene(app->scene_manager, UkfeSceneLog);
        break;
    }
    case MenuItemLog:
        scene_manager_next_scene(app->scene_manager, UkfeSceneLog);
        break;
    case MenuItemSettings:
        scene_manager_next_scene(app->scene_manager, UkfeSceneSettings);
        break;
    }
}

void scene_menu_on_enter(void* ctx) {
    LораUkfeApp* app = ctx;

    static const char* mode_names[] = {"LoRa (Heltec)", "WiFi (T-Dongle)", "SubGHz OOK", "Direkt"};
    const char* mode = (app->mode < UkfeModeCount) ? mode_names[app->mode] : "?";

    submenu_reset(app->submenu);

    // Titelzeile zeigt aktuellen Modus
    char title[32];
    snprintf(title, sizeof(title), "UKFE — %s", mode);
    submenu_set_header(app->submenu, title);

    submenu_add_item(app->submenu, "Status",       MenuItemStatus,   menu_callback, app);
    submenu_add_item(app->submenu, "Trigger",      MenuItemTrigger,  menu_callback, app);
    submenu_add_item(app->submenu, "Payloads",     MenuItemPayloads, menu_callback, app);
    submenu_add_item(app->submenu, "LoRa Scan",    MenuItemLoRaScan, menu_callback, app);
    submenu_add_item(app->submenu, "WiFi Scan",    MenuItemWiFiScan, menu_callback, app);
    submenu_add_item(app->submenu, "RF: Status-Ping (868)", MenuItemRfPing, menu_callback, app);
    submenu_add_item(app->submenu, "RF: HID Marker (868)",  MenuItemRfHid0, menu_callback, app);
    submenu_add_item(app->submenu, "RF: HID Win+R (868)",   MenuItemRfHid1, menu_callback, app);
    submenu_add_item(app->submenu, "Log",          MenuItemLog,      menu_callback, app);
    submenu_add_item(app->submenu, "Einstellungen",MenuItemSettings, menu_callback, app);

    view_dispatcher_switch_to_view(app->view_dispatcher, UkfeViewMenu);
    ukfe_uart_send_status(app);
}

bool scene_menu_on_event(void* ctx, SceneManagerEvent event) {
    UNUSED(ctx);
    UNUSED(event);
    return false;
}

void scene_menu_on_exit(void* ctx) {
    LораUkfeApp* app = ctx;
    submenu_reset(app->submenu);
}
