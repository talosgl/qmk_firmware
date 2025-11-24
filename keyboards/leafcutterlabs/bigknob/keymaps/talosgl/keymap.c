/* Copyright 2021 Craig Gardner
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include QMK_KEYBOARD_H

#define _MAIN 0

// -------- on startup

// void keyboard_post_init_user(void) {
//     // Turn RGB ON at startup with low brightness to test
//     rgblight_enable();
//     rgblight_mode(RGBLIGHT_MODE_STATIC_LIGHT); // Static mode
//     rgblight_sethsv(0, 255, 50);               // Red color, full saturation, low brightness (50/255)
// }

// -------- custom enum function things
enum {
    TD_ENCODER_BTN = 0,
    TD_HUE_CHANGE  = 1,
    TD_BRIGHT_UP   = 2,
    TD_BRIGHT_DOWN = 3,
    TD_RGB         = 4,
};

// -------- knob

// Track if encoder button is being held
static bool encoder_button_held = false;

void dance_encoder_finished(tap_dance_state_t *state, void *user_data) {
    if (state->pressed) {
        // Button is being held
        encoder_button_held = true;
    }
}

void dance_encoder_reset(tap_dance_state_t *state, void *user_data) {
    // Button released
    encoder_button_held = false;

    // If it was a single tap (not a hold), send F24
    if (state->count == 1 && !state->interrupted) {
        tap_code(KC_F24);
    }
}

bool encoder_update_user(uint8_t index, bool clockwise) {
    if (index == 0) {
        if (encoder_button_held) {
            // While holding encoder button: change hue (It also changes volume no matter what)
            if (clockwise) {
                rgblight_increase_val();
            } else {
                rgblight_decrease_val();
            }
        } else {
            // Normal: volume control
            if (clockwise) {
                tap_code(KC_VOLU);
            } else {
                tap_code(KC_VOLD);
            }
        }
    }
    return true;
}

// -------- RGB tapdance stuff

void dance_hue_change_finished(tap_dance_state_t *state, void *user_data) {
    if (state->count == 1) {
        tap_code(KC_F13);
    } else if (state->count == 2) {
        rgblight_increase_hue(); // Change color instead of mode
    }
}

void dance_bright_up_finished(tap_dance_state_t *state, void *user_data) {
    if (state->count == 1) {
        tap_code(KC_F14); // Single tap still sends F14
    } else if (state->count == 2) {
        rgblight_increase_val(); // Double tap increases brightness
    }
}

void dance_bright_down_finished(tap_dance_state_t *state, void *user_data) {
    if (state->count == 1) {
        tap_code(KC_F15); // Single tap still sends F15
    } else if (state->count == 2) {
        rgblight_decrease_val(); // Double tap decreases brightness
    }
}

void dance_rgb_finished(tap_dance_state_t *state, void *user_data) {
    if (state->count == 1) {
        tap_code(KC_F16);
    } else if (state->count == 2) {
        rgblight_toggle();
    } else if (state->count == 3) {
        rgblight_step();
    } else if (state->count == 4) {
        rgblight_mode(RGBLIGHT_MODE_RAINBOW_SWIRL);
    }
}

// All tap dance functions
tap_dance_action_t tap_dance_actions[] = {[TD_ENCODER_BTN] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, dance_encoder_finished, dance_encoder_reset), [TD_HUE_CHANGE] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, dance_hue_change_finished, NULL), [TD_BRIGHT_UP] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, dance_bright_up_finished, NULL), [TD_BRIGHT_DOWN] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, dance_bright_down_finished, NULL), [TD_RGB] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, dance_rgb_finished, NULL)};

// -------- actually set the keymap
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    // button closest to USB is first
    [_MAIN] = LAYOUT(TD(TD_ENCODER_BTN), // Encoder button - tap for F24, turn without hold for volume, hold while turning for brightness up/down (AND volume)
                     TD(TD_HUE_CHANGE),  // F13 on single tap, hue up on double-tap
                     TD(TD_BRIGHT_UP),   // F14 on single tap, brightness up on double tap
                     TD(TD_BRIGHT_DOWN), // F15 on single tap, brightness down on double tap
                     TD(TD_RGB))         // F16 on single tap, RGB toggle on double, cycle animation mode on triple, go directly to rainbow swirl animation on quad-tap
};

/* If I want to swap to using this instead of AHK for the Alt-code keys, basic idea would be:
enum custom_keycodes {
    ALT_CHECKMARK = SAFE_RANGE,
    ALT_GUILLEMET_L,
    ALT_GUILLEMET_R,
    ALT_EM_DASH
};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case ALT_CHECKMARK:
            if (record->event.pressed) {
                register_code(KC_LALT);
                tap_code(KC_KP_2);
                tap_code(KC_KP_5);
                tap_code(KC_KP_1);
                unregister_code(KC_LALT);
            }
            return false;
        // ... etc for other symbols
    }
    return true;
}
*/