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
    MenuItemRfPl0,
    MenuItemRfPl1,
    MenuItemRfPl2,
    MenuItemRfPl3,
    MenuItemRfPl4,
    MenuItemNetPing,
    MenuItemNetPl0,
    MenuItemNetPl1,
    MenuItemNetPl2,
    MenuItemNetPl3,
    MenuItemNetPl4,
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
    case MenuItemRfPl0:
    case MenuItemRfPl1:
    case MenuItemRfPl2:
    case MenuItemRfPl3:
    case MenuItemRfPl4: {
        // id 0..4 -> Heltec UkfeRfControl-Tabelle:
        // 0 wifi_scan_ap · 1 wifi_deauth_all · 2 ble_spam_all · 3 wifi_evil_portal · 4 wifi_wardrive
        uint8_t id = (uint8_t)(index - MenuItemRfPl0);
        UkfeRfMessage m;
        ukfe_rf_make_trigger(&m, id, 0);
        if(rf_comm_init()) rf_comm_send(&m);
        scene_manager_next_scene(app->scene_manager, UkfeSceneLog);
        break;
    }
    case MenuItemNetPing: {
        // Status-Ping ueber GPIO-UART an den WROOM-Relay (-> ESP-NOW an Satelliten)
        UkfeRfMessage m;
        ukfe_rf_make_simple(&m, UkfeRfCmdStatus);
        ukfe_uart_send_rf(app, &m);
        scene_manager_next_scene(app->scene_manager, UkfeSceneLog);
        break;
    }
    case MenuItemNetPl0:
    case MenuItemNetPl1:
    case MenuItemNetPl2:
    case MenuItemNetPl3:
    case MenuItemNetPl4: {
        // gleiche id-Tabelle wie RF, aber Transport = UART -> WROOM -> ESP-NOW
        uint8_t id = (uint8_t)(index - MenuItemNetPl0);
        UkfeRfMessage m;
        ukfe_rf_make_trigger(&m, id, 0);
        ukfe_uart_send_rf(app, &m);
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

    submenu_reset(app->submenu);
    submenu_set_header(app->submenu, "G4MEOVER UKFE");

    // --- 868-FSK: Flipper-CC1101 direkt zum Heltec ---
    submenu_add_item(app->submenu, "RF: Status-Ping (868)", MenuItemRfPing, menu_callback, app);
    submenu_add_item(app->submenu, "RF: WiFi Scan (868)",   MenuItemRfPl0, menu_callback, app);
    submenu_add_item(app->submenu, "RF: WiFi Deauth (868)", MenuItemRfPl1, menu_callback, app);
    submenu_add_item(app->submenu, "RF: BLE Spam (868)",    MenuItemRfPl2, menu_callback, app);
    submenu_add_item(app->submenu, "RF: Evil Portal (868)", MenuItemRfPl3, menu_callback, app);
    submenu_add_item(app->submenu, "RF: Wardrive (868)",    MenuItemRfPl4, menu_callback, app);
    // --- NET: ueber GPIO-UART an den WROOM-Relay -> ESP-NOW an alle Satelliten ---
    submenu_add_item(app->submenu, "NET: Status-Ping (WROOM)", MenuItemNetPing, menu_callback, app);
    submenu_add_item(app->submenu, "NET: WiFi Scan (WROOM)",   MenuItemNetPl0, menu_callback, app);
    submenu_add_item(app->submenu, "NET: WiFi Deauth (WROOM)", MenuItemNetPl1, menu_callback, app);
    submenu_add_item(app->submenu, "NET: BLE Spam (WROOM)",    MenuItemNetPl2, menu_callback, app);
    submenu_add_item(app->submenu, "NET: Evil Portal (WROOM)", MenuItemNetPl3, menu_callback, app);
    submenu_add_item(app->submenu, "NET: Wardrive (WROOM)",    MenuItemNetPl4, menu_callback, app);
    submenu_add_item(app->submenu, "Log",          MenuItemLog,      menu_callback, app);

    view_dispatcher_switch_to_view(app->view_dispatcher, UkfeViewMenu);
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
