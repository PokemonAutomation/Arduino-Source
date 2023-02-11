/*  Scalar Button Framework
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *  Don't use these unless you know what you're doing and you really need to.
 *  These are hard to use and can cause unexpected behavior if used incorrectly.
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_Commands_ScalarButtons_H
#define PokemonAutomation_NintendoSwitch_Commands_ScalarButtons_H

#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"
#include "ClientSource/Connection/BotBase.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


void ssf_flush_pipeline (BotBaseContext& context);
void ssf_do_nothing     (BotBaseContext& context, uint16_t ticks);



void ssf_press_button(
    BotBaseContext& context,
    Button button,
    uint16_t delay, uint16_t hold = 5, uint8_t cool = 3
);
void ssf_press_dpad(
    BotBaseContext& context,
    DpadPosition position,
    uint16_t delay, uint16_t hold = 5, uint8_t cool = 3
);


void ssf_press_left_joystick(
    BotBaseContext& context,
    uint8_t x, uint8_t y,
    uint16_t delay, uint16_t hold, uint8_t cool = 0
);
void ssf_press_right_joystick(
    BotBaseContext& context,
    uint8_t x, uint8_t y,
    uint16_t delay, uint16_t hold, uint8_t cool = 0
);


void ssf_mash1_button   (BotBaseContext& context, Button button, uint16_t ticks);
void ssf_mash2_button   (BotBaseContext& context, Button button0, Button button1, uint16_t ticks);
void ssf_mash_AZs       (BotBaseContext& context, uint16_t ticks);


#define ssf_ScrollDirection     uint16_t
#define SSF_SCROLL_UP           0   //  Diagonal scrolling seems to count as
#define SSF_SCROLL_RIGHT        2   //  seperate events for each direction.
#define SSF_SCROLL_DOWN         4
#define SSF_SCROLL_LEFT         6
void ssf_issue_scroll(
    BotBaseContext& context,
    ssf_ScrollDirection direction,
    uint16_t delay, uint16_t hold = 5, uint8_t cool = 3
);





}
}
#endif
