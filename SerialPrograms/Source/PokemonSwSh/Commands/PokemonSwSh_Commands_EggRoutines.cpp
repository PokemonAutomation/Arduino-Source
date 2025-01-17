/*  Egg Routines
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#include "ClientSource/Libraries/MessageConverter.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "PokemonSwSh_Commands_EggRoutines.h"
//#include "PokemonSwSh_Messages_EggRoutines.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


void eggfetcher_loop(ControllerContext& context){
#if 0
    context.issue_request(
        DeviceRequest_eggfetcher_loop()
    );
#else
    ssf_press_left_joystick(context, STICK_MAX, STICK_MAX, 50, 50);
    ssf_press_left_joystick(context, STICK_MAX, 160, 0, 510);
    ssf_press_button(context, BUTTON_A, 170);
    ssf_press_button(context, BUTTON_A, 170);
    ssf_press_button(context, BUTTON_A, 170);
    ssf_press_left_joystick(context, STICK_MIN, STICK_CENTER, 50, 390);
    ssf_mash1_button(context, BUTTON_B, 390);

    ssf_press_left_joystick(context, 192, STICK_MIN, 120, 120);
    ssf_press_left_joystick(context, STICK_MAX, STICK_MIN, 120, 120);
#endif
}
void move_while_mashing_B(ControllerContext& context, uint16_t duration){
#if 0
    context.issue_request(
        DeviceRequest_move_while_mashing_B(duration)
    );
#else
    //  Hold the joystick to the right for the entire duration.
    ssf_press_left_joystick(context, STICK_MAX, STICK_CENTER, 0, duration);

    //  While the above is running, spam B.
    ssf_mash1_button(context, BUTTON_B, duration);
#endif
}
void spin_and_mash_A(ControllerContext& context, uint16_t duration){
#if 0
    context.issue_request(
        DeviceRequest_spin_and_mash_A(duration)
    );
#else
    for (uint16_t c = 0; c < duration; c += 128){
        ssf_press_left_joystick(context, STICK_CENTER, STICK_MAX, 0, 32);
        ssf_press_button(context, BUTTON_A, 16);
        ssf_press_button(context, BUTTON_A, 16);

        ssf_press_left_joystick(context, STICK_MAX, STICK_CENTER, 0, 32);
        ssf_press_button(context, BUTTON_A, 16);
        ssf_press_button(context, BUTTON_A, 16);

        ssf_press_left_joystick(context, STICK_CENTER, STICK_MIN, 0, 32);
        ssf_press_button(context, BUTTON_A, 16);
        ssf_press_button(context, BUTTON_A, 16);

        ssf_press_left_joystick(context, STICK_MIN, STICK_CENTER, 0, 32);
        ssf_press_button(context, BUTTON_A, 16);
        ssf_press_button(context, BUTTON_A, 16);
    }
#endif
}
void travel_to_spin_location(ControllerContext& context){
#if 0
    context.issue_request(
        DeviceRequest_travel_to_spin_location()
    );
#else
    ssf_press_left_joystick(context, STICK_MAX, 144, 100, 250);
    ssf_press_button(context, BUTTON_A, 100);
    ssf_press_button(context, BUTTON_A, 50);
    ssf_press_left_joystick(context, STICK_MAX, STICK_MAX, 50, 50);
#endif
}
void travel_back_to_lady(ControllerContext& context){
#if 0
    context.issue_request(
        DeviceRequest_travel_back_to_lady()
    );
#else
    ssf_press_left_joystick(context, STICK_CENTER, STICK_MAX, 30, 30);
    ssf_press_left_joystick(context, STICK_MAX, 144, 260, 260);
    ssf_press_left_joystick(context, STICK_MIN, STICK_CENTER, 50, 400);
    ssf_mash1_button(context, BUTTON_B, 400);
    ssf_press_left_joystick(context, 192, STICK_MIN, 120, 120);
    ssf_press_left_joystick(context, STICK_MAX, STICK_MIN, 120, 120);
#endif
}


#if 0
int register_message_converters_eggs(){
    register_message_converter(
        PABB_MSG_COMMAND_EGG_FETCHER_LOOP,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "eggfetcher_loop() - ";
            if (body.size() != sizeof(pabb_eggfetcher_loop)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_eggfetcher_loop*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_COMMAND_MOVE_WHILE_MASHING_B,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "move_while_mashing_B() - ";
            if (body.size() != sizeof(pabb_move_while_mashing_B)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_move_while_mashing_B*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", duration = " << params->duration;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_COMMAND_SPIN_AND_MASH_A,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "spin_and_mash_A() - ";
            if (body.size() != sizeof(pabb_spin_and_mash_A)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_spin_and_mash_A*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            ss << ", duration = " << params->duration;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_COMMAND_TRAVEL_TO_SPIN_LOCATION,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "travel_to_spin_location() - ";
            if (body.size() != sizeof(pabb_travel_to_spin_location)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_travel_to_spin_location*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            return ss.str();
        }
    );
    register_message_converter(
        PABB_MSG_COMMAND_TRAVEL_BACK_TO_LADY,
        [](const std::string& body){
            std::ostringstream ss;
            ss << "travel_back_to_lady() - ";
            if (body.size() != sizeof(pabb_travel_back_to_lady)){ ss << "(invalid size)" << std::endl; return ss.str(); }
            const auto* params = (const pabb_travel_back_to_lady*)body.c_str();
            ss << "seqnum = " << (uint64_t)params->seqnum;
            return ss.str();
        }
    );
    return 0;
}
int init_PokemonSwShEggRoutines = register_message_converters_eggs();
#endif


}
}

