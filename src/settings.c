/** Copyright 2011-2012 Thorsten Wißmann. All rights reserved.
 *
 * This software is licensed under the "Simplified BSD License".
 * See LICENSE for details */


#include "settings.h"
#include "clientlist.h"
#include "layout.h"
#include "ipc-protocol.h"
#include "utils.h"

#include <glib.h>
#include <string.h>
#include <stdio.h>

#define SET_INT(NAME, DEFAULT, CALLBACK) \
    { .name = (NAME), \
      .value = { .i = (DEFAULT) }, \
      .type = HS_Int, \
      .on_change = (CALLBACK), \
    }

#define SET_STRING(NAME, DEFAULT, CALLBACK) \
    { .name = (NAME), \
      .value = { .s = (DEFAULT) }, \
      .type = HS_String, \
      .on_change = (CALLBACK), \
    }

// often used callbacks:
#define RELAYOUT all_monitors_apply_layout
#define FR_COLORS reset_frame_colors
#define CL_COLORS reset_client_colors

// default settings:
SettingsPair g_settings[] = {
    SET_INT(    "window_gap",                      5,           RELAYOUT    ),
    SET_INT(    "snap_distance",                   10,          NULL        ),
    SET_INT(    "snap_gap",                        5,           NULL        ),
    SET_STRING( "border_color",                    "red",       NULL        ),
    SET_STRING( "frame_border_active_color",       "red",       FR_COLORS   ),
    SET_STRING( "frame_border_normal_color",       "blue",      FR_COLORS   ),
    SET_STRING( "frame_bg_normal_color",           "green",     FR_COLORS   ),
    SET_STRING( "frame_bg_active_color",           "green",     FR_COLORS   ),
    SET_INT(    "frame_bg_transparent",            0,           FR_COLORS   ),
    SET_INT(    "frame_border_width",              2,           FR_COLORS   ),
    SET_INT(    "window_border_width",             2,           CL_COLORS   ),
    SET_STRING( "window_border_active_color",      "red",       CL_COLORS   ),
    SET_STRING( "window_border_normal_color",      "blue",      CL_COLORS   ),
    SET_INT(    "always_show_frame",               0,           RELAYOUT    ),
    SET_INT(    "default_direction_external_only", 0,           NULL        ),
    SET_INT(    "default_frame_layout",            0,           FR_COLORS   ),
    SET_INT(    "focus_follows_shift",             1,           NULL        ),
    SET_INT(    "focus_follows_mouse",             0,           NULL        ),
    SET_INT(    "focus_stealing_prevention",       1,           NULL        ),
    SET_INT(    "swap_monitors_to_get_tag",        1,           NULL        ),
    SET_INT(    "raise_on_focus",                  1,           NULL        ),
    SET_INT(    "raise_on_click",                  1,           NULL        ),
    SET_INT(    "gapless_grid",                    1,           RELAYOUT    ),
    SET_STRING( "tree_style",                      "*| +`--.",  FR_COLORS   ),
};

int settings_count() {
    return LENGTH(g_settings);
}

void settings_init() {
    // recreate all strings -> move them to heap
    int i;
    for (i = 0; i < LENGTH(g_settings); i++) {
        if (g_settings[i].type == HS_String) {
            g_settings[i].value.s = g_strdup(g_settings[i].value.s);
        }
    }
}

void settings_destroy() {
    // free all strings
    int i;
    for (i = 0; i < LENGTH(g_settings); i++) {
        if (g_settings[i].type == HS_String) {
            g_free(g_settings[i].value.s);
        }
    }
}


SettingsPair* settings_find(char* name) {
    int i;
    for (i = 0; i < LENGTH(g_settings); i++) {
        if (!strcmp(g_settings[i].name, name)) {
            return g_settings + i;
        }
    }
    return NULL;
}

int settings_set(int argc, char** argv) {
    if (argc < 3) {
        return HERBST_INVALID_ARGUMENT;
    }
    SettingsPair* pair = settings_find(argv[1]);
    if (!pair) {
        return HERBST_SETTING_NOT_FOUND;
    }
    if (pair->type == HS_Int) {
        int new_value;
        // parse value to int, if possible
        if (1 == sscanf(argv[2], "%d", &new_value)) {
            if (new_value == pair->value.i) {
                // nothing would be changed
                return 0;
            }
            pair->value.i = new_value;
        } else {
            return HERBST_INVALID_ARGUMENT;
        }
    } else { // pair->type == HS_String
        if (!strcmp(pair->value.s, argv[2])) {
            // nothing would be changed
            return 0;
        }
        g_free(pair->value.s);
        pair->value.s = g_strdup(argv[2]);
    }
    // on successfull change, call callback
    if (pair->on_change) {
        pair->on_change();
    }
    return 0;
}

int settings_get(int argc, char** argv, GString** output) {
    if (argc < 2) {
        return HERBST_INVALID_ARGUMENT;
    }
    SettingsPair* pair = settings_find(argv[1]);
    if (!pair) {
        return HERBST_SETTING_NOT_FOUND;
    }
    if (pair->type == HS_Int) {
        g_string_printf(*output, "%d", pair->value.i);
    } else { // pair->type == HS_String
        *output = g_string_assign(*output, pair->value.s);
    }
    return 0;
}


// toggle integer-like values
int settings_toggle(int argc, char** argv) {
    if (argc < 2) {
        return HERBST_INVALID_ARGUMENT;
    }
    SettingsPair* pair = settings_find(argv[1]);
    if (!pair) {
        return HERBST_SETTING_NOT_FOUND;
    }
    if (pair->type == HS_Int) {
        pair->value.i = !pair->value.i;
    } else {
        // only toggle numbers
        return HERBST_INVALID_ARGUMENT;
    }
    // on successfull change, call callback
    if (pair->on_change) {
        pair->on_change();
    }
    return 0;
}

