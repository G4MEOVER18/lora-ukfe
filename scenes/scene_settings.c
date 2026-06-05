#include "../lora_ukfe.h"
#include "scenes.h"

static const char* mode_labels[]   = {"LoRa (Heltec)", "WiFi (T-Dongle)", "SubGHz OOK", "Direkt"};
static const char* lora_profiles[] = {"G4MEOVER 868.1", "DogyTag 868.0"};

static void mode_change(VariableItem* item) {
    LораUkfeApp* app  = variable_item_get_context(item);
    uint8_t idx       = variable_item_get_current_value_index(item);
    app->mode         = (UkfeMode)idx;
    variable_item_set_current_value_text(item, mode_labels[idx]);
}

void scene_settings_on_enter(void* ctx) {
    LораUkfeApp* app = ctx;
    variable_item_list_reset(app->var_list);

    VariableItem* item;

    // Kommunikationsmodus
    item = variable_item_list_add(app->var_list, "Modus", UkfeModeCount, mode_change, app);
    variable_item_set_current_value_index(item, (uint8_t)app->mode);
    variable_item_set_current_value_text(item, mode_labels[app->mode]);

    // LoRa-Profil (nur Info, wird per JSON-Befehl gesetzt)
    item = variable_item_list_add(app->var_list, "LoRa-Profil", 2, NULL, app);
    variable_item_set_current_value_index(item, 0);
    variable_item_set_current_value_text(item, lora_profiles[0]);

    view_dispatcher_switch_to_view(app->view_dispatcher, UkfeViewSettings);
}

bool scene_settings_on_event(void* ctx, SceneManagerEvent event) {
    UNUSED(ctx);
    bool consumed = false;
    if(event.type == SceneManagerEventTypeBack) {
        consumed = true;
    }
    return consumed;
}

void scene_settings_on_exit(void* ctx) {
    LораUkfeApp* app = ctx;
    variable_item_list_reset(app->var_list);
}
