#include "lora_ukfe.h"
#include "scenes/scenes.h"
#include <string.h>
#include <furi.h>
#include <gui/gui.h>
#include <notification/notification.h>

// ─── Log-Buffer ───────────────────────────────────────────────────────────────

#define LOG_MAX (UKFE_JSON_BUF_SIZE * 4)

void ukfe_log_append(LораUkfeApp* app, const char* line) {
    furi_mutex_acquire(app->log_mutex, FuriWaitForever);

    size_t cur_len = strlen(app->log_buf);
    size_t line_len = strlen(line);
    const size_t max = LOG_MAX - 2;

    // Wenn Buffer zu voll: älteste Zeile oben abschneiden
    if(cur_len + line_len + 1 >= max) {
        const char* nl = strchr(app->log_buf, '\n');
        if(nl) {
            size_t skip = (size_t)(nl - app->log_buf) + 1;
            memmove(app->log_buf, app->log_buf + skip, cur_len - skip + 1);
            cur_len -= skip;
        } else {
            app->log_buf[0] = '\0';
            cur_len = 0;
        }
    }

    strlcat(app->log_buf, line, max);
    strlcat(app->log_buf, "\n", max);

    furi_mutex_release(app->log_mutex);
    view_dispatcher_send_custom_event(app->view_dispatcher, 0xFF);
}

// ─── Back-Callback ────────────────────────────────────────────────────────────

static bool view_dispatcher_custom_event_cb(void* ctx, uint32_t event) {
    LораUkfeApp* app = ctx;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

static bool view_dispatcher_back_event_cb(void* ctx) {
    LораUkfeApp* app = ctx;
    return scene_manager_handle_back_event(app->scene_manager);
}

static void view_dispatcher_tick_event_cb(void* ctx) {
    LораUkfeApp* app = ctx;
    scene_manager_handle_tick_event(app->scene_manager);
}

// ─── App-Alloc / Free ─────────────────────────────────────────────────────────

static LораUkfeApp* app_alloc(void) {
    LораUkfeApp* app = malloc(sizeof(LораUkfeApp));
    memset(app, 0, sizeof(LораUkfeApp));

    app->mode          = UkfeModeLora;
    app->log_mutex     = furi_mutex_alloc(FuriMutexTypeNormal);
    app->notifications = furi_record_open(RECORD_NOTIFICATION);

    // GUI
    Gui* gui = furi_record_open(RECORD_GUI);

    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_custom_event_callback(app->view_dispatcher, view_dispatcher_custom_event_cb);
    view_dispatcher_set_navigation_event_callback(app->view_dispatcher, view_dispatcher_back_event_cb);
    view_dispatcher_set_tick_event_callback(app->view_dispatcher, view_dispatcher_tick_event_cb, 100);
    view_dispatcher_attach_to_gui(app->view_dispatcher, gui, ViewDispatcherTypeFullscreen);

    app->scene_manager = scene_manager_alloc(&ukfe_scene_handlers, app);

    // Views
    app->submenu = submenu_alloc();
    view_dispatcher_add_view(app->view_dispatcher, UkfeViewMenu,
        submenu_get_view(app->submenu));

    app->text_box = text_box_alloc();
    view_dispatcher_add_view(app->view_dispatcher, UkfeViewStatus,
        text_box_get_view(app->text_box));

    app->log_box = text_box_alloc();
    view_dispatcher_add_view(app->view_dispatcher, UkfeViewLog,
        text_box_get_view(app->log_box));

    app->var_list = variable_item_list_alloc();
    view_dispatcher_add_view(app->view_dispatcher, UkfeViewSettings,
        variable_item_list_get_view(app->var_list));

    return app;
}

static void app_free(LораUkfeApp* app) {
    ukfe_uart_deinit(app);

    view_dispatcher_remove_view(app->view_dispatcher, UkfeViewMenu);
    view_dispatcher_remove_view(app->view_dispatcher, UkfeViewStatus);
    view_dispatcher_remove_view(app->view_dispatcher, UkfeViewLog);
    view_dispatcher_remove_view(app->view_dispatcher, UkfeViewSettings);

    submenu_free(app->submenu);
    text_box_free(app->text_box);
    text_box_free(app->log_box);
    variable_item_list_free(app->var_list);

    scene_manager_free(app->scene_manager);
    view_dispatcher_free(app->view_dispatcher);

    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_NOTIFICATION);
    furi_mutex_free(app->log_mutex);
    free(app);
}

// ─── Entry Point ──────────────────────────────────────────────────────────────

int32_t lora_ukfe_app(void* p) {
    UNUSED(p);

    LораUkfeApp* app = app_alloc();

    if(!ukfe_uart_init(app)) {
        ukfe_log_append(app, "[UART] Init fehlgeschlagen!");
    }

    // Startbildschirm: Modus-Auswahl
    scene_manager_next_scene(app->scene_manager, UkfeSceneModeMenu);
    view_dispatcher_run(app->view_dispatcher);

    app_free(app);
    return 0;
}
