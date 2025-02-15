/*  Superscalar Framework
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
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
        &context, button,
        delay*8ms, hold*8ms, cool*8ms
    );
}
void ssf_press_button(
    ProControllerContext& context,
    Button button,
    Milliseconds delay, Milliseconds hold, Milliseconds cool
){
    context->issue_buttons(&context, button, delay, hold, cool);
}

void ssf_press_dpad(
    ProControllerContext& context,
    DpadPosition position,
    uint16_t delay, uint16_t hold, uint8_t cool
){
    context->issue_dpad(
        &context, position,
        delay*8ms, hold*8ms, cool*8ms
    );
}
void ssf_press_dpad(
    ProControllerContext& context,
    DpadPosition position,
    Milliseconds delay, Milliseconds hold, Milliseconds cool
){
    context->issue_dpad(&context, position, delay, hold, cool);
}

void ssf_press_left_joystick(
    ProControllerContext& context,
    uint8_t x, uint8_t y,
    uint16_t delay, uint16_t hold, uint8_t cool
){
    context->issue_left_joystick(
        &context, x, y,
        delay*8ms, hold*8ms, cool*8ms
    );
}
void ssf_press_left_joystick(
    ProControllerContext& context,
    uint8_t x, uint8_t y,
    Milliseconds delay, Milliseconds hold, Milliseconds cool
){
    context->issue_left_joystick(&context, x, y, delay, hold, cool);
}
void ssf_press_right_joystick(
    ProControllerContext& context,
    uint8_t x, uint8_t y,
    uint16_t delay, uint16_t hold, uint8_t cool
){
    context->issue_right_joystick(
        &context, x, y,
        delay*8ms, hold*8ms, cool*8ms
    );
}
void ssf_press_right_joystick(
    ProControllerContext& context,
    uint8_t x, uint8_t y,
    Milliseconds delay, Milliseconds hold, Milliseconds cool
){
    context->issue_right_joystick(&context, x, y, delay, hold, cool);
}



void ssf_mash1_button(ProControllerContext& context, Button button, uint16_t ticks){
    context->issue_mash_button(&context, button, ticks*8ms);
}
void ssf_mash1_button(ProControllerContext& context, Button button, Milliseconds duration){
    context->issue_mash_button(&context, button, duration);
}
void ssf_mash2_button(ProControllerContext& context, Button button0, Button button1, uint16_t ticks){
    context->issue_mash_button(&context, button0, button1, ticks*8ms);
}
void ssf_mash2_button(ProControllerContext& context, Button button0, Button button1, Milliseconds duration){
    context->issue_mash_button(&context, button0, button1, duration);
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
        &context, direction,
        delay*8ms, hold*8ms, cool*8ms
    );
}
void ssf_issue_scroll(
    ProControllerContext& context,
    DpadPosition direction,
    Milliseconds delay, Milliseconds hold, Milliseconds cool
){
    context->issue_system_scroll(&context, direction, delay, hold, cool);
}





void ssf_press_button(ProControllerContext& context, Button button){
    Milliseconds delay = 3 * 8ms;
    Milliseconds hold = 5 * 8ms;
    Milliseconds cool = 3 * 8ms;
    Milliseconds timing_variation = context->timing_variation();
    delay += 2 * timing_variation;
    hold += timing_variation;
    cool += timing_variation;
    ssf_press_button(context, button, delay, hold, cool);
}
void ssf_issue_scroll(ProControllerContext& context, DpadPosition direction){
    Milliseconds delay = 3 * 8ms;
    Milliseconds hold = 5 * 8ms;
    Milliseconds cool = 3 * 8ms;
    Milliseconds timing_variation = context->timing_variation();
    delay += 2 * timing_variation;
    hold += timing_variation;
    cool += timing_variation;
    ssf_issue_scroll(context, direction, delay, hold, cool);
}







}
}
