/*  Superscalar Framework (ssf_*)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *  Don't use these unless you know what you're doing and you really need to.
 *
 *  If you don't understand how the state machine works, you will not be able
 *  to properly use these functions.
 *
 *  Since the functionality here used to be internal-only, documentation remains
 *  sparse for now and slated to be expanded in the future.
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_Commands_Superscalar_H
#define PokemonAutomation_NintendoSwitch_Commands_Superscalar_H

#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_Controller.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


void ssf_flush_pipeline (ControllerContext& context);
void ssf_do_nothing     (ControllerContext& context, uint16_t ticks);



void ssf_press_button(
    ControllerContext& context,
    Button button,
    uint16_t delay, uint16_t hold = 5, uint8_t cool = 3
);
void ssf_press_dpad(
    ControllerContext& context,
    DpadPosition position,
    uint16_t delay, uint16_t hold = 5, uint8_t cool = 3
);


void ssf_press_left_joystick(
    ControllerContext& context,
    uint8_t x, uint8_t y,
    uint16_t delay, uint16_t hold, uint8_t cool = 0
);
void ssf_press_right_joystick(
    ControllerContext& context,
    uint8_t x, uint8_t y,
    uint16_t delay, uint16_t hold, uint8_t cool = 0
);


void ssf_mash1_button   (ControllerContext& context, Button button, uint16_t ticks);
void ssf_mash2_button   (ControllerContext& context, Button button0, Button button1, uint16_t ticks);
void ssf_mash_AZs       (ControllerContext& context, uint16_t ticks);


#define ssf_ScrollDirection     uint16_t
#define SSF_SCROLL_UP           0   //  Diagonal scrolling seems to count as
#define SSF_SCROLL_RIGHT        2   //  seperate events for each direction.
#define SSF_SCROLL_DOWN         4
#define SSF_SCROLL_LEFT         6
void ssf_issue_scroll(
    ControllerContext& context,
    ssf_ScrollDirection direction,
    uint16_t delay, uint16_t hold = 5, uint8_t cool = 3
);





}
}
#endif
