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


void ssf_flush_pipeline(SwitchControllerContext& context){
    context->issue_barrier(&context);
}
void ssf_do_nothing(SwitchControllerContext& context, uint16_t ticks){
    context->issue_nop(&context, ticks*8ms);
}
void ssf_do_nothing(SwitchControllerContext& context, Milliseconds duration){
    context->issue_nop(&context, duration);
}


void ssf_press_button(
    SwitchControllerContext& context,
    Button button,
    uint16_t delay, uint16_t hold, uint8_t cool
){
    context->issue_buttons(
        &context, button,
        delay*8ms, hold*8ms, cool*8ms
    );
}
void ssf_press_button(
    SwitchControllerContext& context,
    Button button,
    Milliseconds delay, Milliseconds hold, Milliseconds cool
){
    context->issue_buttons(&context, button, delay, hold, cool);
}

void ssf_press_dpad(
    SwitchControllerContext& context,
    DpadPosition position,
    uint16_t delay, uint16_t hold, uint8_t cool
){
    context->issue_dpad(
        &context, position,
        delay*8ms, hold*8ms, cool*8ms
    );
}
void ssf_press_dpad(
    SwitchControllerContext& context,
    DpadPosition position,
    Milliseconds delay, Milliseconds hold, Milliseconds cool
){
    context->issue_dpad(&context, position, delay, hold, cool);
}

void ssf_press_left_joystick(
    SwitchControllerContext& context,
    uint8_t x, uint8_t y,
    uint16_t delay, uint16_t hold, uint8_t cool
){
    context->issue_left_joystick(
        &context, x, y,
        delay*8ms, hold*8ms, cool*8ms
    );
}
void ssf_press_left_joystick(
    SwitchControllerContext& context,
    uint8_t x, uint8_t y,
    Milliseconds delay, Milliseconds hold, Milliseconds cool
){
    context->issue_left_joystick(&context, x, y, delay, hold, cool);
}
void ssf_press_right_joystick(
    SwitchControllerContext& context,
    uint8_t x, uint8_t y,
    uint16_t delay, uint16_t hold, uint8_t cool
){
    context->issue_right_joystick(
        &context, x, y,
        delay*8ms, hold*8ms, cool*8ms
    );
}
void ssf_press_right_joystick(
    SwitchControllerContext& context,
    uint8_t x, uint8_t y,
    Milliseconds delay, Milliseconds hold, Milliseconds cool
){
    context->issue_right_joystick(&context, x, y, delay, hold, cool);
}



void ssf_mash1_button(SwitchControllerContext& context, Button button, uint16_t ticks){
    context->issue_mash_button(&context, button, ticks*8ms);
}
void ssf_mash1_button(SwitchControllerContext& context, Button button, Milliseconds duration){
    context->issue_mash_button(&context, button, duration);
}
void ssf_mash2_button(SwitchControllerContext& context, Button button0, Button button1, uint16_t ticks){
    context->issue_mash_button(&context, button0, button1, ticks*8ms);
}
void ssf_mash2_button(SwitchControllerContext& context, Button button0, Button button1, Milliseconds duration){
    context->issue_mash_button(&context, button0, button1, duration);
}
void ssf_mash_AZs(SwitchControllerContext& context, uint16_t ticks){
    context->issue_mash_AZs(&context, ticks*8ms);
}
void ssf_mash_AZs(SwitchControllerContext& context, Milliseconds duration){
    context->issue_mash_AZs(&context, duration);
}
void ssf_issue_scroll(
    SwitchControllerContext& context,
    DpadPosition direction,
    uint16_t delay, uint16_t hold, uint8_t cool
){
    context->issue_system_scroll(
        &context, direction,
        delay*8ms, hold*8ms, cool*8ms
    );
}
void ssf_issue_scroll(
    SwitchControllerContext& context,
    DpadPosition direction,
    Milliseconds delay, Milliseconds hold, Milliseconds cool
){
    context->issue_system_scroll(&context, direction, delay, hold, cool);
}



#if 0
int register_message_converters_ssf(){
    register_message_converter(
        PABB_MSG_COMMAND_SSF_FLUSH_PIPELINE,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "ssf_flush_pipeline() - ";
            if (body.size() != sizeof(pabb_ssf_flush_pipeline)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_ssf_flush_pipeline*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_COMMAND_SSF_DO_NOTHING,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "ssf_do_nothing() - ";
            if (body.size() != sizeof(pabb_ssf_do_nothing)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_ssf_do_nothing*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", ticks = " << params->ticks;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_COMMAND_SSF_PRESS_BUTTON,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "ssf_press_button() - ";
            if (body.size() != sizeof(pabb_ssf_press_button)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_ssf_press_button*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", button = " << params->button << "(" << button_to_string(params->button) << ")";
            ss << ", delay = " << params->delay;
            ss << ", hold = " << params->hold;
            ss << ", cool = " << (int)params->cool;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_COMMAND_SSF_PRESS_DPAD,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "ssf_press_dpad() - ";
            if (body.size() != sizeof(pabb_ssf_press_dpad)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_ssf_press_dpad*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", dpad = " << (unsigned)params->position << "(" << dpad_to_string(params->position) << ")";
            ss << ", delay = " << params->delay;
            ss << ", hold = " << params->hold;
            ss << ", cool = " << (int)params->cool;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_COMMAND_SSF_PRESS_JOYSTICK_L,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "ssf_press_joystick(left) - ";
            if (body.size() != sizeof(pabb_ssf_press_joystick)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_ssf_press_joystick*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", x = " << (int)params->x;
            ss << ", y = " << (int)params->y;
            ss << ", delay = " << params->delay;
            ss << ", hold = " << params->hold;
            ss << ", cool = " << (int)params->cool;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_COMMAND_SSF_PRESS_JOYSTICK_R,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "ssf_press_joystick(right) - ";
            if (body.size() != sizeof(pabb_ssf_press_joystick)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_ssf_press_joystick*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", x = " << (int)params->x;
            ss << ", y = " << (int)params->y;
            ss << ", delay = " << params->delay;
            ss << ", hold = " << params->hold;
            ss << ", cool = " << (int)params->cool;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_COMMAND_SSF_MASH1_BUTTON,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "ssf_mash1_button() - ";
            if (body.size() != sizeof(pabb_ssf_mash1_button)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_ssf_mash1_button*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", button = " << params->button << "(" << button_to_string(params->button) << ")";
            ss << ", ticks = " << params->ticks;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_COMMAND_SSF_MASH2_BUTTON,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "ssf_mash2_button() - ";
            if (body.size() != sizeof(pabb_ssf_mash2_button)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_ssf_mash2_button*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", button = " << params->button0 << "(" << button_to_string(params->button0) << ")";
            ss << ", button = " << params->button1 << "(" << button_to_string(params->button1) << ")";
            ss << ", ticks = " << params->ticks;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_COMMAND_SSF_MASH_AZS,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "ssf_mash_AZs() - ";
            if (body.size() != sizeof(pabb_ssf_mash_AZs)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_ssf_mash_AZs*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", ticks = " << params->ticks;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_COMMAND_SSF_SCROLL,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "ssf_issue_scroll() - ";
            if (body.size() != sizeof(pabb_ssf_issue_scroll)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_ssf_issue_scroll*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", delay = " << params->delay;
            ss << ", hold = " << params->hold;
            ss << ", cool = " << (int)params->cool;
            return ss.str();
        }
    );
    return 0;
}
int init_ssf = register_message_converters_ssf();
#endif



}
}
