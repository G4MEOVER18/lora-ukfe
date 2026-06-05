#include "../lora_ukfe.h"
#include "scenes.h"

void scene_log_on_enter(void* ctx) {
    LораUkfeApp* app = ctx;
    text_box_reset(app->log_box);
    text_box_set_font(app->log_box, TextBoxFontText);

    furi_mutex_acquire(app->log_mutex, FuriWaitForever);
    text_box_set_text(app->log_box, app->log_buf);
    furi_mutex_release(app->log_mutex);

    text_box_set_focus(app->log_box, TextBoxFocusEnd);
    view_dispatcher_switch_to_view(app->view_dispatcher, UkfeViewLog);
}

bool scene_log_on_event(void* ctx, SceneManagerEvent event) {
    LораUkfeApp* app = ctx;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        furi_mutex_acquire(app->log_mutex, FuriWaitForever);
        text_box_set_text(app->log_box, app->log_buf);
        furi_mutex_release(app->log_mutex);
        text_box_set_focus(app->log_box, TextBoxFocusEnd);
        consumed = true;
    } else if(event.type == SceneManagerEventTypeBack) {
        scene_manager_previous_scene(app->scene_manager);
        consumed = true;
    }
    return consumed;
}

void scene_log_on_exit(void* ctx) {
    LораUkfeApp* app = ctx;
    text_box_reset(app->log_box);
}
