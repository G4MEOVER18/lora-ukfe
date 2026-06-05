#include "../lora_ukfe.h"
#include "scenes.h"

// ─── Scene-Handler-Tabelle ────────────────────────────────────────────────────
// Reihenfolge muss exakt der UkfeScene-Enum-Reihenfolge entsprechen!

static void (*const on_enter_handlers[])(void*) = {
    [UkfeSceneMenu]           = scene_menu_on_enter,
    [UkfeSceneModeMenu]       = scene_mode_menu_on_enter,
    [UkfeSceneStatus]         = scene_status_on_enter,
    [UkfeScenePayloadList]    = scene_payload_list_on_enter,
    [UkfeSceneTriggerConfirm] = scene_status_on_enter,
    [UkfeSceneLog]            = scene_log_on_enter,
    [UkfeSceneSettings]       = scene_settings_on_enter,
};

static bool (*const on_event_handlers[])(void*, SceneManagerEvent) = {
    [UkfeSceneMenu]           = scene_menu_on_event,
    [UkfeSceneModeMenu]       = scene_mode_menu_on_event,
    [UkfeSceneStatus]         = scene_status_on_event,
    [UkfeScenePayloadList]    = scene_payload_list_on_event,
    [UkfeSceneTriggerConfirm] = scene_status_on_event,
    [UkfeSceneLog]            = scene_log_on_event,
    [UkfeSceneSettings]       = scene_settings_on_event,
};

static void (*const on_exit_handlers[])(void*) = {
    [UkfeSceneMenu]           = scene_menu_on_exit,
    [UkfeSceneModeMenu]       = scene_mode_menu_on_exit,
    [UkfeSceneStatus]         = scene_status_on_exit,
    [UkfeScenePayloadList]    = scene_payload_list_on_exit,
    [UkfeSceneTriggerConfirm] = scene_status_on_exit,
    [UkfeSceneLog]            = scene_log_on_exit,
    [UkfeSceneSettings]       = scene_settings_on_exit,
};

const SceneManagerHandlers ukfe_scene_handlers = {
    .on_enter_handlers = on_enter_handlers,
    .on_event_handlers = on_event_handlers,
    .on_exit_handlers  = on_exit_handlers,
    .scene_num         = UkfeSceneCount,
};
