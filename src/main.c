#include <gbdk/platform.h>
#include <stdint.h>
#include <stdbool.h>
#include <rand.h>
#include <gb/crash_handler.h>


#include "gbc_hicolor.h"
#include "rng_data.h"

// GBC HiColor images; header file names align with png file names
//#include "gbbsod.h"
//#include "gbbsod_grad.h"
#include "gbbsod_smallqr.h"
//#include "gbbsod_smallqr_grad.h"
#include "gb_win9xbsod.h"
#include "gbbsod_win8.h"
#include "gbbsod_longhorn.h"
#include "macosx_panic.h"
#include "macos_classic.h"
#include "gbstudio_panic.h"
#include "amiga_meditate.h"


#define BG_LAST_TILE  255u
const uint8_t blank_tile[] = {0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0};

#define ARRAY_LEN(A)  (sizeof(A) / sizeof(A[0]))

uint8_t buttons, buttons_prev;
#define UPDATE_BUTTONS()            (buttons_prev = buttons, buttons = joypad())
#define BUTTON_TOGGLED(BUTTON_MASK) ((buttons & (~buttons_prev)) & (BUTTON_MASK))
#define BUTTON_PRESSED(BUTTON_MASK) (buttons & (BUTTON_MASK))

typedef struct far_ptr_t {
    uint8_t bank;
    const void * ptr;
} far_ptr_t;

// Array of pointers to the generated hicolor data structures
const far_ptr_t hicolors[] = {
    { BANK(gbbsod_smallqr),         &HICOLOR_VAR(gbbsod_smallqr) },
    { BANK(gbbsod_win8),            &HICOLOR_VAR(gbbsod_win8) },
    { BANK(gb_win9xbsod),           &HICOLOR_VAR(gb_win9xbsod) },
    { BANK(gbbsod_longhorn),        &HICOLOR_VAR(gbbsod_longhorn) },
    { BANK(macos_classic),          &HICOLOR_VAR(macos_classic) },
    { BANK(macosx_panic),           &HICOLOR_VAR(macosx_panic) },
    { BANK(gbstudio_panic),         &HICOLOR_VAR(gbstudio_panic) },
    { BANK(amiga_meditate),         &HICOLOR_VAR(amiga_meditate) },
//    { BANK(gbbsod_smallqr_grad),    &HICOLOR_VAR(gbbsod_smallqr_grad) },
//    { BANK(gbbsod_grad),            &HICOLOR_VAR(gbbsod_grad) },
//    { BANK(gbbsod),                 &HICOLOR_VAR(gbbsod) },
};


void check_game_boy_color(void) {

    if (_cpu != CGB_TYPE) {
        // If it's not a Game Boy Color then...
        __HandleCrash();

        // Could yolo it and crash with style to the crash handler ;D
        // void (*lets_gooooo)(void) = (void (*)(void))randw();
        // lets_gooooo();
    }
}


void main(void) {

    check_game_boy_color();

    uint8_t  img_select;
    // Image toggling variable, by default show a random entry
    // If SRAM RNG data is not initialized then show the default first image (0)
    rng_load();
    if (rng_is_initialized())  img_select = (rand() + 1u) % (uint8_t)ARRAY_LEN(hicolors);
    else img_select = 0u;
    rng_save();

    bool     first_pass = true;
    uint8_t  scroll_limit = 0;
    const    hicolor_data * p_hicolor;
    uint8_t  hicolor_bank;

    SHOW_BKG;

    // Require Game Boy Color
    if (_cpu == CGB_TYPE) {
        // CGB running in the double speed mode is required
        cpu_fast();

        while(true) {

            vsync();
            UPDATE_BUTTONS();

            // Change displayed Hi Color image when pressing A or B
            if (BUTTON_TOGGLED(J_A | J_B | J_LEFT | J_RIGHT) || first_pass) {

                // Mix user input into the RNG and save it to SRAM
                rng_mix();

                // Cycle through which image to show next
                if (BUTTON_TOGGLED(J_A | J_RIGHT)) {
                    img_select++;
                    if (img_select == ARRAY_LEN(hicolors)) img_select = 0;
                }
                else if BUTTON_TOGGLED(J_B | J_LEFT) {
                    if (img_select == 0) img_select = ARRAY_LEN(hicolors) - 1;
                    else img_select--;
                }

                vsync();
                DISPLAY_OFF;

                // Set current image to show
                hicolor_bank = hicolors[img_select].bank;
                p_hicolor = (const hicolor_data *)hicolors[img_select].ptr;

                uint8_t bank_save = CURRENT_BANK;
                if (hicolor_bank) SWITCH_ROM(hicolor_bank);

                // Reset Y scroll and set scroll range based on converted image height
                SCY_REG = 0u;
                if ((p_hicolor->height_in_tiles * 8u) > DEVICE_SCREEN_PX_HEIGHT)
                    scroll_limit = ((p_hicolor->height_in_tiles * 8u) - DEVICE_SCREEN_PX_HEIGHT);
                else scroll_limit = 0;

                // Optional:
                // If the Hi Color image is shorter than screen height
                // then fill the remaining screen area with a tile.
                //
                // Put the tile at the end of CGB tile pattern vram since
                // the short Hi Color image will be too small to use all of it.
                if ((p_hicolor->height_in_tiles * 8u) < DEVICE_SCREEN_PX_HEIGHT) {
                    VBK_REG = VBK_BANK_1;
                    set_bkg_data(BG_LAST_TILE, 1u, blank_tile);
                    fill_bkg_rect(0u, (p_hicolor->height_in_tiles), DEVICE_SCREEN_WIDTH, DEVICE_SCREEN_HEIGHT, BKGF_BANK1);
                    VBK_REG = VBK_BANK_0;
                    fill_bkg_rect(0u, (p_hicolor->height_in_tiles), DEVICE_SCREEN_WIDTH, DEVICE_SCREEN_HEIGHT, BG_LAST_TILE);
                }

                SWITCH_ROM(bank_save);

                // Load and display the HiColor image
                hicolor_start(p_hicolor, hicolor_bank);

                DISPLAY_ON;

                first_pass = false;
            }
            // Scroll Up/Down if available
            else if (BUTTON_PRESSED(J_UP)) {
                if (SCY_REG) SCY_REG--;
            } else if (BUTTON_PRESSED(J_DOWN)) {
                if (SCY_REG < scroll_limit) SCY_REG++;
            }
        }
    }
}

