#include <gbdk/platform.h>

#define UPDATE_BUTTONS()            (buttons_prev = buttons, buttons = joypad())
#define BUTTON_TOGGLED(BUTTON_MASK) ((buttons & (~buttons_prev)) & (BUTTON_MASK))
#define BUTTON_PRESSED(BUTTON_MASK) (buttons & (BUTTON_MASK))
#define J_A_B_SEL_START (J_A | J_B | J_SELECT | J_START)
#define J_ANY           (J_A | J_B | J_SELECT | J_START | J_UP | J_DOWN | J_LEFT | J_RIGHT)

extern uint8_t buttons, buttons_prev;