#include "../lora_ukfe.h"
#include "scenes.h"

static void mode_callback(void* ctx, uint32_t index) {
    LораUkfeApp* app = ctx;
    app->mode = (UkfeMode)index;
    scene_manager_next_scene(app->scene_manager, UkfeSceneMenu);
}

void scene_mode_menu_on_enter(void* ctx) {
    LораUkfeApp* app = ctx;

    submenu_reset(app->submenu);
    submenu_set_header(app->submenu, "USB Army Knife FE");

    submenu_add_item(app->submenu, "LoRa (Heltec v3)",  UkfeModeLora,   mode_callback, app);
    submenu_add_item(app->submenu, "WiFi (T-Dongle S2)", UkfeModeWifi,   mode_callback, app);
    submenu_add_item(app->submenu, "SubGHz OOK (CC1101)",UkfeModeSubGhz, mode_callback, app);
    submenu_add_item(app->submenu, "Direktmodus (UART)", UkfeModeDirect, mode_callback, app);

    submenu_set_selected_item(app->submenu, (uint32_t)app->mode);
    view_dispatcher_switch_to_view(app->view_dispatcher, UkfeViewMenu);
}

bool scene_mode_menu_on_event(void* ctx, SceneManagerEvent event) {
    UNUSED(ctx);
    UNUSED(event);
    return false;
}

void scene_mode_menu_on_exit(void* ctx) {
    LораUkfeApp* app = ctx;
    submenu_reset(app->submenu);
}
