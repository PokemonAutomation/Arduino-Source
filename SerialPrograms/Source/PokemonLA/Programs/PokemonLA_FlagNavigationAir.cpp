/*  Flag Navigation (Air)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Tools/ConsoleHandle.h"
#include "CommonFramework/Tools/InterruptableCommands.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonLA_FlagNavigationAir.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


FlagNavigationAir::FlagNavigationAir(ProgramEnvironment& env, ConsoleHandle& console)
    : SuperControlSession(env, console)
    , m_flag(console, console)
    , m_mount(console)
    , m_centerA(console, console, ButtonType::ButtonA, {0.40, 0.50, 0.40, 0.50}, std::chrono::milliseconds(200), false)
    , m_leftB(console, console, ButtonType::ButtonB, {0.02, 0.40, 0.05, 0.20}, std::chrono::milliseconds(200), false)
    , m_looking_straight_ahead(false)
{
    *this += m_flag;
    *this += m_mount;
    *this += m_centerA;
    *this += m_leftB;
    register_state_command(State::UNKNOWN, [=](){
        m_console.log("Unknown state. Moving foward...");
        m_active_command->dispatch([=](const BotBaseContext& context){
            pbf_move_left_joystick(context, 128, 0, 300 * TICKS_PER_SECOND, 0);
        });
        return false;
    });
    register_state_command(State::WYRDEER_BASCULEGION_OFF, [=](){
        m_console.log("Switching from Wyrdeer/Basculegion (off) to Braviary (on)...");
        m_active_command->dispatch([=](const BotBaseContext& context){
            pbf_press_dpad(context, DPAD_RIGHT, 20, 50);
            pbf_press_button(context, BUTTON_PLUS, 20, GET_ON_BRAVIARY_TIME);
        });
        return false;
    });
    register_state_command(State::WYRDEER_BASCULEGION_ON, [=](){
        m_console.log("Switching from Wyrdeer/Basculegion (on) to Braviary (on)...");
        m_active_command->dispatch([=](const BotBaseContext& context){
            pbf_press_dpad(context, DPAD_RIGHT, 20, GET_ON_BRAVIARY_TIME);
        });
        return false;
    });
    register_state_command(State::URSALUNA_OFF, [=](){
        m_console.log("Switching from Ursaluna (off) to Braviary (on)...");
        m_active_command->dispatch([=](const BotBaseContext& context){
            pbf_press_dpad(context, DPAD_RIGHT, 20, 50);
            pbf_press_dpad(context, DPAD_RIGHT, 20, 50);
            pbf_press_button(context, BUTTON_PLUS, 20, GET_ON_BRAVIARY_TIME);
        });
        return false;
    });
    register_state_command(State::URSALUNA_ON, [=](){
        m_console.log("Switching from Ursaluna (on) to Braviary (on)...");
        m_active_command->dispatch([=](const BotBaseContext& context){
            pbf_press_dpad(context, DPAD_RIGHT, 20, 50);
            pbf_press_dpad(context, DPAD_RIGHT, 20, GET_ON_BRAVIARY_TIME);
        });
        return false;
    });
    register_state_command(State::SNEASLER_OFF, [=](){
        m_console.log("Switching from Sneasler (off) to Braviary (on)...");
        m_active_command->dispatch([=](const BotBaseContext& context){
            pbf_press_dpad(context, DPAD_LEFT, 20, 50);
            pbf_press_button(context, BUTTON_PLUS, 20, GET_ON_BRAVIARY_TIME);
        });
        return false;
    });
    register_state_command(State::SNEASLER_ON, [=](){
        m_console.log("Switching from Sneasler (on) to Braviary (on)...");
        m_active_command->dispatch([=](const BotBaseContext& context){
            pbf_move_left_joystick(context, 128, 0, 125, 0);
            pbf_press_dpad(context, DPAD_LEFT, 20, GET_ON_BRAVIARY_TIME);
        });
        m_looking_straight_ahead = false;
        return false;
    });
    register_state_command(State::BRAVIARY_OFF, [=](){
        m_console.log("Switching from Braviary (off) to Braviary (on)...");
        m_active_command->dispatch([=](const BotBaseContext& context){
            pbf_press_button(context, BUTTON_PLUS, 20, GET_ON_BRAVIARY_TIME);
        });
        return false;
    });
    register_state_command(State::GET_ON_SNEASLER, [=](){
        m_console.log("Getting on Sneasler...");
        m_active_command->dispatch([=](const BotBaseContext& context){
            pbf_press_button(context, BUTTON_A, 20, 230);
        });
        m_looking_straight_ahead = false;
        return false;
    });
    register_state_command(State::CLIMBING, [=](){
        m_console.log("Climbing wall...");
        m_active_command->dispatch([=](const BotBaseContext& context){
            pbf_move_left_joystick(context, 128, 0, 300 * TICKS_PER_SECOND, 0);
        });
        m_looking_straight_ahead = false;
        return false;
    });
    register_state_command(State::DASH_FORWARD, [=](){
        m_console.log("Dashing forward...");
        bool looking_straight_ahead = m_looking_straight_ahead;
        m_active_command->dispatch([=](const BotBaseContext& context){
            //  Move forward to straighten out direction.
            if (!looking_straight_ahead){
                pbf_move_left_joystick(context, 128, 0, 125, 0);
            }
            pbf_mash_button(context, BUTTON_B, 300 * TICKS_PER_SECOND);
        });
        m_looking_straight_ahead = true;
        return false;
    });
    register_state_command(State::DASH_LEFT, [=](){
        m_console.log("Dashing Left...");
        bool looking_straight_ahead = m_looking_straight_ahead;
        m_active_command->dispatch([=](const BotBaseContext& context){
            //  Move forward to straighten out direction.
            if (!looking_straight_ahead){
                pbf_move_left_joystick(context, 128, 0, 125, 0);
            }
            pbf_press_button(context, BUTTON_B, 50, 0);
            pbf_controller_state(context, BUTTON_B, DPAD_NONE, 96, 128, 128, 128, 255);
        });
        m_looking_straight_ahead = true;
        return false;
    });
    register_state_command(State::DASH_RIGHT, [=](){
        m_console.log("Dashing Right...");
        bool looking_straight_ahead = m_looking_straight_ahead;
        m_active_command->dispatch([=](const BotBaseContext& context){
            //  Move forward to straighten out direction.
            if (!looking_straight_ahead){
                pbf_move_left_joystick(context, 128, 0, 125, 0);
            }
            pbf_press_button(context, BUTTON_B, 50, 0);
            pbf_controller_state(context, BUTTON_B, DPAD_NONE, 160, 128, 128, 128, 255);
        });
        m_looking_straight_ahead = true;
        return false;
    });
    register_state_command(State::TURN_LEFT, [=](){
        m_console.log("Turning Left...");
        m_active_command->dispatch([=](const BotBaseContext& context){
            pbf_wait(context, 125);
            pbf_move_right_joystick(context, 0, 128, (uint16_t)(85 * (0.5 - m_flag_x)), 0);
            pbf_move_left_joystick(context, 128, 0, 125, 125);
        });
        m_looking_straight_ahead = false;
        return false;
    });
    register_state_command(State::TURN_RIGHT, [=](){
        m_console.log("Turning Right...");
        m_active_command->dispatch([=](const BotBaseContext& context){
            pbf_wait(context, 125);
            pbf_move_right_joystick(context, 255, 128, (uint16_t)(85 * (m_flag_x - 0.5)), 0);
            pbf_move_left_joystick(context, 128, 0, 125, 125);
        });
        m_looking_straight_ahead = false;
        return false;
    });
    register_state_command(State::FIND_FLAG, [=](){
        m_console.log("Looking for flag...");
        m_active_command->dispatch([=](const BotBaseContext& context){
            pbf_move_right_joystick(context, 128, 255, 200, 0);
            pbf_move_right_joystick(context, 128, 0, 200, 0);
            pbf_move_right_joystick(context, 128, 255, 80, 0);
            pbf_move_right_joystick(context, 0, 128, 400, 0);
            pbf_move_right_joystick(context, 128, 255, 120, 0);
            pbf_move_right_joystick(context, 0, 128, 400, 0);
            pbf_move_right_joystick(context, 128, 0, 200, 0);
            pbf_move_right_joystick(context, 0, 128, 400, 0);
        });
        m_looking_straight_ahead = false;
        return false;
    });
}

bool FlagNavigationAir::run_state(
    AsyncCommandSession& commands,
    std::chrono::system_clock::time_point timestamp
){
    switch (m_mount.state()){
    case MountState::NOTHING:
        return run_state_action(State::UNKNOWN);
    case MountState::WYRDEER_OFF:
    case MountState::BASCULEGION_OFF:
        return run_state_action(State::WYRDEER_BASCULEGION_OFF);
    case MountState::WYRDEER_ON:
    case MountState::BASCULEGION_ON:
        return run_state_action(State::WYRDEER_BASCULEGION_ON);
    case MountState::URSALUNA_OFF:
        return run_state_action(State::URSALUNA_OFF);
    case MountState::URSALUNA_ON:
        return run_state_action(State::URSALUNA_ON);
    case MountState::SNEASLER_OFF:
        return run_state_action(State::SNEASLER_OFF);
    case MountState::SNEASLER_ON:
        return run_climbing(commands, timestamp);
    case MountState::BRAVIARY_OFF:
        return run_state_action(State::BRAVIARY_OFF);
    case MountState::BRAVIARY_ON:
        return run_flying(commands, timestamp);
    }

    m_console.log("No state handler for current state.", COLOR_RED);
    return false;
}
bool FlagNavigationAir::run_flying(AsyncCommandSession& commands, WallClock timestamp){
    double flag_distance, flag_y;
    bool flag_ok = m_flag.get(flag_distance, m_flag_x, flag_y);
//    cout << "flag_ok = " << flag_ok << ", x = " << flag_x << ", y = " << flag_y << endl;

    //  Need to get on Sneasler.
    if (m_centerA.detected()){
        return run_state_action(State::GET_ON_SNEASLER);
    }

    //  Cruise
    if (flag_ok && flag_y <= 0.6 && 0.45 <= m_flag_x && m_flag_x <= 0.55){
        return run_state_action(State::DASH_FORWARD);
    }

    //  Turning Cruise
    if (flag_ok && flag_y <= 0.6 && 0.40 <= m_flag_x && m_flag_x <= 0.45){
        return run_state_action(State::DASH_LEFT);
    }
    if (flag_ok && flag_y <= 0.6 && 0.55 <= m_flag_x && m_flag_x <= 0.60){
        return run_state_action(State::DASH_RIGHT);
    }

    //  Re-center the flag.
    if (flag_ok && m_flag_x <= 0.4){
        return run_state_action(State::TURN_LEFT);
    }
    if (flag_ok && m_flag_x >= 0.6){
        return run_state_action(State::TURN_RIGHT);
    }

    //  Find the flag.
    if (!flag_ok){
        return run_state_action(State::FIND_FLAG);
    }

    return false;
}
bool FlagNavigationAir::run_climbing(AsyncCommandSession& commands, WallClock timestamp){
    //  Can't jump off means you're able to stand. Switch back to Braviary.
    if (!m_leftB.detected()){
        return run_state_action(State::SNEASLER_ON);
    }
    return run_state_action(State::CLIMBING);
}



}
}
}
