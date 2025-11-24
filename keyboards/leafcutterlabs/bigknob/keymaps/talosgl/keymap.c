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
// -------- Custom keycodes
enum custom_keycodes { ENC_TOGGLE = SAFE_RANGE };

// -------- Tap dance enum
enum {
    TD_HUE_CHANGE  = 0,
    TD_BRIGHT_UP   = 1,
    TD_BRIGHT_DOWN = 2,
    TD_RGB         = 3,
};

// -------- knob

// -------- Encoder state tracking
static bool encoder_button_held = false;
static bool encoder_was_used    = false;

// -------- Encoder button handling
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (keycode == ENC_TOGGLE) {
        if (record->event.pressed) {
            encoder_button_held = true;
        } else {
            encoder_button_held = false;

            // Only toggle RGB if encoder wasn't used for brightness
            if (!encoder_was_used) {
                rgblight_toggle();
            }
            encoder_was_used = false;
        }
        return false;
    }
    return true;
}

// -------- Encoder rotation handling
bool encoder_update_user(uint8_t index, bool clockwise) {
    if (index == 0) {
        if (encoder_button_held) {
            encoder_was_used = true;
            clockwise ? rgblight_increase_val() : rgblight_decrease_val();
        } else {
            clockwise ? tap_code(KC_VOLU) : tap_code(KC_VOLD);
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

// All tap dance actions
tap_dance_action_t tap_dance_actions[] = {[TD_HUE_CHANGE] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, dance_hue_change_finished, NULL), [TD_BRIGHT_UP] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, dance_bright_up_finished, NULL), [TD_BRIGHT_DOWN] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, dance_bright_down_finished, NULL), [TD_RGB] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, dance_rgb_finished, NULL)};

// -------- Keymap
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {[_MAIN] = LAYOUT(ENC_TOGGLE,         // Tap to toggle RGB, hold+turn for brightness
                                                                               TD(TD_HUE_CHANGE),  // F13 / double-tap hue up
                                                                               TD(TD_BRIGHT_UP),   // F14 / double-tap brightness up
                                                                               TD(TD_BRIGHT_DOWN), // F15 / double-tap brightness down
                                                                               TD(TD_RGB)          // F16 / double-tap RGB toggle / triple cycle / quad rainbow
                                                                               )};

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

// -------- on startup

// void keyboard_post_init_user(void) {
//     // Turn RGB ON at startup with low brightness to test
//     rgblight_enable();
//     rgblight_mode(RGBLIGHT_MODE_STATIC_LIGHT); // Static mode
//     rgblight_sethsv(0, 255, 50);               // Red color, full saturation, low brightness (50/255)
// }