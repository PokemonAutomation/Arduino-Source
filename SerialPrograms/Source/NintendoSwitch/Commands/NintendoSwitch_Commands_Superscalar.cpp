/*  Superscalar Framework
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

//#include <sstream>
#include "ClientSource/Libraries/MessageConverter.h"
#include "NintendoSwitch_Commands_Superscalar.h"
//#include "NintendoSwitch_Messages_Superscalar.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


void ssf_flush_pipeline(ProControllerContext& context){
    context->issue_barrier(&context);
}
void ssf_do_nothing(ProControllerContext& context, uint16_t ticks){
    context->issue_nop(&context, ticks*8ms);
}
void ssf_do_nothing(ProControllerContext& context, Milliseconds duration){
    context->issue_nop(&context, duration);
}


void ssf_press_button(
    ProControllerContext& context,
    Button button,
    uint16_t delay, uint16_t hold, uint8_t cool
){
    context->issue_buttons(
        &context,
        delay*8ms, hold*8ms, cool*8ms,
        button
    );
}
void ssf_press_button(
    ProControllerContext& context,
    Button button,
    Milliseconds delay, Milliseconds hold, Milliseconds cool
){
    context->issue_buttons(&context, delay, hold, cool, button);
}

void ssf_press_dpad(
    ProControllerContext& context,
    DpadPosition position,
    uint16_t delay, uint16_t hold, uint8_t cool
){
    context->issue_dpad(
        &context,
        delay*8ms, hold*8ms, cool*8ms,
        position
    );
}
void ssf_press_dpad(
    ProControllerContext& context,
    DpadPosition position,
    Milliseconds delay, Milliseconds hold, Milliseconds cool
){
    context->issue_dpad(&context, delay, hold, cool, position);
}

void ssf_press_left_joystick(
    ProControllerContext& context,
    uint8_t x, uint8_t y,
    uint16_t delay, uint16_t hold, uint8_t cool
){
    context->issue_left_joystick(
        &context,
        delay*8ms, hold*8ms, cool*8ms,
        x, y
    );
}
void ssf_press_left_joystick(
    ProControllerContext& context,
    uint8_t x, uint8_t y,
    Milliseconds delay, Milliseconds hold, Milliseconds cool
){
    context->issue_left_joystick(&context, delay, hold, cool, x, y);
}
void ssf_press_right_joystick(
    ProControllerContext& context,
    uint8_t x, uint8_t y,
    uint16_t delay, uint16_t hold, uint8_t cool
){
    context->issue_right_joystick(
        &context,
        delay*8ms, hold*8ms, cool*8ms,
        x, y
    );
}
void ssf_press_right_joystick(
    ProControllerContext& context,
    uint8_t x, uint8_t y,
    Milliseconds delay, Milliseconds hold, Milliseconds cool
){
    context->issue_right_joystick(&context, delay, hold, cool, x, y);
}



void ssf_mash1_button(ProControllerContext& context, Button button, uint16_t ticks){
    context->issue_mash_button(&context, ticks*8ms, button);
}
void ssf_mash1_button(ProControllerContext& context, Button button, Milliseconds duration){
    context->issue_mash_button(&context, duration, button);
}
void ssf_mash2_button(ProControllerContext& context, Button button0, Button button1, uint16_t ticks){
    context->issue_mash_button(&context, ticks*8ms, button0, button1);
}
void ssf_mash2_button(ProControllerContext& context, Button button0, Button button1, Milliseconds duration){
    context->issue_mash_button(&context, duration, button0, button1);
}
void ssf_mash_AZs(ProControllerContext& context, uint16_t ticks){
    context->issue_mash_AZs(&context, ticks*8ms);
}
void ssf_mash_AZs(ProControllerContext& context, Milliseconds duration){
    context->issue_mash_AZs(&context, duration);
}
void ssf_issue_scroll(
    ProControllerContext& context,
    DpadPosition direction,
    uint16_t delay, uint16_t hold, uint8_t cool
){
    context->issue_system_scroll(
        &context,
        delay*8ms, hold*8ms, cool*8ms,
        direction
    );
}
void ssf_issue_scroll(
    ProControllerContext& context,
    DpadPosition direction,
    Milliseconds delay
){
    context->issue_system_scroll(&context, delay, 2*delay, delay, direction);
}
void ssf_issue_scroll(
    ProControllerContext& context,
    DpadPosition direction,
    Milliseconds delay, Milliseconds hold, Milliseconds cool
){
    context->issue_system_scroll(&context, delay, hold, cool, direction);
}




void ssf_flush_pipeline(JoyconContext& context){
    context->issue_barrier(&context);
}
void ssf_do_nothing(JoyconContext& context, Milliseconds duration){
    context->issue_nop(&context, duration);
}
void ssf_press_button(
    JoyconContext& context,
    Button button,
    Milliseconds delay, Milliseconds hold, Milliseconds cool
){
    context->issue_buttons(&context, button, delay, hold, cool);
}
void ssf_press_joystick(
    JoyconContext& context,
    uint8_t x, uint8_t y,
    Milliseconds delay, Milliseconds hold, Milliseconds cool
){
    context->issue_joystick(&context, x, y, delay, hold, cool);
}
void ssf_mash1_button(JoyconContext& context, Button button, Milliseconds duration){
    context->issue_mash_button(&context, button, duration);
}
void ssf_issue_scroll(
    JoyconContext& context,
    DpadPosition direction,
    Milliseconds delay
){
    ssf_issue_scroll(context, direction, delay, 2*delay, delay);
}
void ssf_issue_scroll(
    JoyconContext& context,
    DpadPosition direction,
    Milliseconds delay, Milliseconds hold, Milliseconds cool
){
    switch (direction){
    case DpadPosition::DPAD_UP:
        ssf_press_joystick(context, 128, 0, delay, hold, cool);
        break;
    case DpadPosition::DPAD_UP_RIGHT:
        ssf_press_joystick(context, 255, 0, delay, hold, cool);
        break;
    case DpadPosition::DPAD_RIGHT:
        ssf_press_joystick(context, 255, 128, delay, hold, cool);
        break;
    case DpadPosition::DPAD_DOWN_RIGHT:
        ssf_press_joystick(context, 255, 255, delay, hold, cool);
        break;
    case DpadPosition::DPAD_DOWN:
        ssf_press_joystick(context, 128, 255, delay, hold, cool);
        break;
    case DpadPosition::DPAD_DOWN_LEFT:
        ssf_press_joystick(context, 0, 255, delay, hold, cool);
        break;
    case DpadPosition::DPAD_LEFT:
        ssf_press_joystick(context, 0, 128, delay, hold, cool);
        break;
    case DpadPosition::DPAD_UP_LEFT:
        ssf_press_joystick(context, 0, 0, delay, hold, cool);
        break;
    default:;
    }
}







}
}
