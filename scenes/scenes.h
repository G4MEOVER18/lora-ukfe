#pragma once

// ─── Scene-IDs ────────────────────────────────────────────────────────────────
// Neue Scene: scene_manager_next_scene(app->scene_manager, UkfeSceneXxx)

typedef enum {
    UkfeSceneMenu = 0,
    UkfeSceneModeMenu,
    UkfeSceneStatus,
    UkfeScenePayloadList,
    UkfeSceneTriggerConfirm,
    UkfeSceneLog,
    UkfeSceneSettings,
    UkfeSceneCount,
} UkfeScene;

// Scene-Handler Deklarationen (je scene_*.c)
extern const SceneManagerHandlers ukfe_scene_handlers;

// ─── scene_menu.c ─────────────────────────────────────────────────────────────
void scene_menu_on_enter(void* ctx);
bool scene_menu_on_event(void* ctx, SceneManagerEvent event);
void scene_menu_on_exit(void* ctx);

// ─── scene_mode_menu.c ────────────────────────────────────────────────────────
void scene_mode_menu_on_enter(void* ctx);
bool scene_mode_menu_on_event(void* ctx, SceneManagerEvent event);
void scene_mode_menu_on_exit(void* ctx);

// ─── scene_status.c ───────────────────────────────────────────────────────────
void scene_status_on_enter(void* ctx);
bool scene_status_on_event(void* ctx, SceneManagerEvent event);
void scene_status_on_exit(void* ctx);

// ─── scene_payload_list.c ─────────────────────────────────────────────────────
void scene_payload_list_on_enter(void* ctx);
bool scene_payload_list_on_event(void* ctx, SceneManagerEvent event);
void scene_payload_list_on_exit(void* ctx);

// ─── scene_log.c ──────────────────────────────────────────────────────────────
void scene_log_on_enter(void* ctx);
bool scene_log_on_event(void* ctx, SceneManagerEvent event);
void scene_log_on_exit(void* ctx);

// ─── scene_settings.c ─────────────────────────────────────────────────────────
void scene_settings_on_enter(void* ctx);
bool scene_settings_on_event(void* ctx, SceneManagerEvent event);
void scene_settings_on_exit(void* ctx);
