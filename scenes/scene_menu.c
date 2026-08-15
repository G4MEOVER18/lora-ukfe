#include "../lora_ukfe.h"
#include "scenes.h"
#include "../rf/rf_comm.h"
#include "../command_catalog.h"

// Hierarchisches Menue: Kategorie-Ebene (s_cat<0) -> Funktions-Ebene (s_cat>=0).
// Kurz scrollen, OK = auswaehlen/ausfuehren, "< zurueck" oder Back = eine Ebene hoch.
static int s_cat = -1;

#define IDX_LOG  200
#define IDX_BACK 201

static void menu_callback(void* ctx, uint32_t index);

static void build_menu(LораUkfeApp* app) {
    submenu_reset(app->submenu);
    if(s_cat < 0) {
        submenu_set_header(app->submenu, "G4MEOVER \u2014 Kategorie");
        for(int i = 0; i < CATALOG_COUNT; i++)
            submenu_add_item(app->submenu, CATALOG[i].name, (uint32_t)i, menu_callback, app);
        submenu_add_item(app->submenu, "Log", IDX_LOG, menu_callback, app);
    } else {
        submenu_set_header(app->submenu, CATALOG[s_cat].name);
        for(uint8_t i = 0; i < CATALOG[s_cat].count; i++)
            submenu_add_item(app->submenu, CATALOG[s_cat].fns[i].name, (uint32_t)i, menu_callback, app);
        submenu_add_item(app->submenu, "< zurueck", IDX_BACK, menu_callback, app);
    }
}

// Baut einen ukfe_rf-Frame aus dem Katalog-Eintrag und sendet ihn ueber die USART.
static void send_fn(LораUkfeApp* app, const UkfeFn* fn) {
    UkfeRfMessage m;
    m.cmd = fn->cmd;
    m.arg_len = fn->has_arg ? 1 : 0;
    if(fn->has_arg) m.args[0] = fn->arg;
    ukfe_uart_send_rf(app, &m);
}

static void menu_callback(void* ctx, uint32_t index) {
    LораUkfeApp* app = ctx;
    if(index == IDX_LOG) { scene_manager_next_scene(app->scene_manager, UkfeSceneLog); return; }
    if(index == IDX_BACK) { s_cat = -1; build_menu(app); return; }
    if(s_cat < 0) {
        if((int)index < CATALOG_COUNT) { s_cat = (int)index; build_menu(app); }
    } else {
        if(index < CATALOG[s_cat].count) {
            send_fn(app, &CATALOG[s_cat].fns[index]);
            scene_manager_next_scene(app->scene_manager, UkfeSceneLog);
        }
    }
}

void scene_menu_on_enter(void* ctx) {
    LораUkfeApp* app = ctx;
    s_cat = -1;
    build_menu(app);
    view_dispatcher_switch_to_view(app->view_dispatcher, UkfeViewMenu);
}

bool scene_menu_on_event(void* ctx, SceneManagerEvent event) {
    LораUkfeApp* app = ctx;
    // Back auf Funktions-Ebene -> zurueck zu den Kategorien (App nicht verlassen).
    if(event.type == SceneManagerEventTypeBack && s_cat >= 0) {
        s_cat = -1;
        build_menu(app);
        return true;
    }
    return false;
}

void scene_menu_on_exit(void* ctx) {
    LораUkfeApp* app = ctx;
    submenu_reset(app->submenu);
}
