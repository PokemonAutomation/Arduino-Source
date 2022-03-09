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


FlagNavigationAir::FlagNavigationAir(
    ProgramEnvironment& env, ConsoleHandle& console,
    bool stop_on_shiny
)
    : SuperControlSession(env, console)
    , m_stop_on_shiny(stop_on_shiny)
    , m_flag(console, console)
    , m_mount(console)
    , m_centerA(console, console, ButtonType::ButtonA, {0.40, 0.50, 0.40, 0.50}, std::chrono::milliseconds(200), false)
    , m_leftB(console, console, ButtonType::ButtonB, {0.02, 0.40, 0.05, 0.20}, std::chrono::milliseconds(200), false)
    , m_shiny_listener(console, false)
    , m_looking_straight_ahead(false)
    , m_last_good_state(WallClock::min())
    , m_find_flag_failed(false)
    , m_last_known_flag_y(0)
{
    *this += m_flag;
    *this += m_mount;
    *this += m_centerA;
    *this += m_leftB;
    register_state_command(State::UNKNOWN, [=](){
        m_console.log("Unknown state. Moving camera around...");
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
        m_looking_straight_ahead.store(false, std::memory_order_release);
        m_active_command->dispatch([=](const BotBaseContext& context){
            pbf_move_left_joystick(context, 128, 0, 125, 0);
            pbf_press_dpad(context, DPAD_LEFT, 20, GET_ON_BRAVIARY_TIME);
        });
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
        m_looking_straight_ahead.store(false, std::memory_order_release);
        m_active_command->dispatch([=](const BotBaseContext& context){
            pbf_press_button(context, BUTTON_A, 20, 230);
        });
        return false;
    });
    register_state_command(State::CLIMBING, [=](){
        m_console.log("Climbing wall...");
        m_looking_straight_ahead.store(false, std::memory_order_release);
        m_active_command->dispatch([=](const BotBaseContext& context){
            pbf_move_left_joystick(context, 128, 0, 300 * TICKS_PER_SECOND, 0);
        });
        return false;
    });
    register_state_command(State::DASH_FORWARD_MASH_B, [=](){
        m_console.log("Dashing forward (mash B)...");
        m_active_command->dispatch([=](const BotBaseContext& context){
            //  Move forward to straighten out direction.
            if (!m_looking_straight_ahead.load(std::memory_order_acquire)){
                pbf_move_left_joystick(context, 128, 0, 160, 0);
                context.wait_for_all_requests();
                m_looking_straight_ahead.store(true, std::memory_order_release);
//                cout << "State::DASH_FORWARD: m_looking_straight_ahead = true" << endl;
            }
            pbf_mash_button(context, BUTTON_B, 300 * TICKS_PER_SECOND);
        });
        return false;
    });
    register_state_command(State::DASH_FORWARD_HOLD_B, [=](){
        m_console.log("Dashing forward (hold B)...");
        m_active_command->dispatch([=](const BotBaseContext& context){
            //  Move forward to straighten out direction.
            if (!m_looking_straight_ahead.load(std::memory_order_acquire)){
                pbf_move_left_joystick(context, 128, 0, 160, 0);
                context.wait_for_all_requests();
                m_looking_straight_ahead.store(true, std::memory_order_release);
//                cout << "State::DASH_FORWARD: m_looking_straight_ahead = true" << endl;
            }
            pbf_press_button(context, BUTTON_B, 300 * TICKS_PER_SECOND, 0);
        });
        return false;
    });
    register_state_command(State::DIVE, [=](){
        m_console.log("Diving...");
        m_active_command->dispatch([=](const BotBaseContext& context){
            //  Move forward to straighten out direction.
            if (!m_looking_straight_ahead.load(std::memory_order_acquire)){
                pbf_move_left_joystick(context, 128, 0, 160, 0);
                context.wait_for_all_requests();
                m_looking_straight_ahead.store(true, std::memory_order_release);
//                cout << "State::DASH_FORWARD: m_looking_straight_ahead = true" << endl;
            }
            pbf_press_button(context, BUTTON_Y, 60 * TICKS_PER_SECOND, 0);
        });
        return false;
    });
    register_state_command(State::DASH_LEFT, [=](){
        m_console.log("Dashing Left...");
        m_active_command->dispatch([=](const BotBaseContext& context){
            //  Move forward to straighten out direction.
            if (!m_looking_straight_ahead.load(std::memory_order_acquire)){
                pbf_move_left_joystick(context, 128, 0, 160, 0);
                context.wait_for_all_requests();
                m_looking_straight_ahead.store(true, std::memory_order_release);
//                cout << "State::DASH_LEFT: m_looking_straight_ahead = true" << endl;
            }
            pbf_press_button(context, BUTTON_B, 50, 0);
            pbf_controller_state(context, BUTTON_B, DPAD_NONE, 96, 128, 128, 128, 255);
        });
        return false;
    });
    register_state_command(State::DASH_RIGHT, [=](){
        m_console.log("Dashing Right...");
        m_active_command->dispatch([=](const BotBaseContext& context){
            //  Move forward to straighten out direction.
            if (!m_looking_straight_ahead.load(std::memory_order_acquire)){
                pbf_move_left_joystick(context, 128, 0, 160, 0);
                context.wait_for_all_requests();
                m_looking_straight_ahead.store(true, std::memory_order_release);
//                cout << "State::DASH_RIGHT: m_looking_straight_ahead = true" << endl;
            }
            pbf_press_button(context, BUTTON_B, 50, 0);
            pbf_controller_state(context, BUTTON_B, DPAD_NONE, 160, 128, 128, 128, 255);
        });
        return false;
    });
    register_state_command(State::TURN_LEFT, [=](){
        m_console.log("Turning Left...");
        m_active_command->dispatch([=](const BotBaseContext& context){
            pbf_wait(context, 125);
            context.wait_for_all_requests();
            double distance, flag_x, flag_y;
            if (m_flag.get(distance, flag_x, flag_y)){
                pbf_move_right_joystick(context, 0, 128, (uint16_t)(85 * (0.5 - flag_x)), 0);
            }
        });
        m_looking_straight_ahead.store(false, std::memory_order_release);
        return false;
    });
    register_state_command(State::TURN_RIGHT, [=](){
        m_console.log("Turning Right...");
        m_active_command->dispatch([=](const BotBaseContext& context){
            pbf_wait(context, 125);
            context.wait_for_all_requests();
            double distance, flag_x, flag_y;
            if (m_flag.get(distance, flag_x, flag_y)){
                pbf_move_right_joystick(context, 255, 128, (uint16_t)(85 * (flag_x - 0.5)), 0);
            }
        });
        m_looking_straight_ahead.store(false, std::memory_order_release);
        return false;
    });
    register_state_command(State::FIND_FLAG, [=](){
        m_console.log("Looking for flag...");
        m_active_command->dispatch([=](const BotBaseContext& context){
            for (size_t c = 0; c < 2; c++){
                pbf_move_right_joystick(context, 128, 255, 200, 0);
                pbf_move_right_joystick(context, 128, 0, 200, 0);
                pbf_move_right_joystick(context, 128, 255, 80, 0);
                pbf_move_right_joystick(context, 0, 128, 400, 0);
                pbf_move_right_joystick(context, 128, 255, 120, 0);
                pbf_move_right_joystick(context, 0, 128, 400, 0);
                pbf_move_right_joystick(context, 128, 0, 200, 0);
                pbf_move_right_joystick(context, 0, 128, 400, 0);
            }
            context.wait_for_all_requests();
            m_find_flag_failed.store(true, std::memory_order_release);
        });
        m_looking_straight_ahead.store(false, std::memory_order_release);
        return false;
    });
}

bool FlagNavigationAir::run_state(
    AsyncCommandSession& commands,
    std::chrono::system_clock::time_point timestamp
){
    if (m_stop_on_shiny && m_shiny_listener.detected()){
        return true;
    }

    if (m_find_flag_failed.load(std::memory_order_acquire)){
        m_console.log("Unable to find flag. Exiting routine...");
        return true;
    }

    m_flag_detected = m_flag.get(m_flag_distance, m_flag_x, m_flag_y);
//    cout << "flag_ok = " << flag_ok << ", x = " << m_flag_x << ", y = " << flag_y << endl;
    if (m_flag_detected){
        m_last_known_flag_y = m_flag_y;
    }

    MountState mount = m_mount.state();
    if (mount == MountState::NOTHING){
        m_console.log("Unable to detect mount. Assuming Braviary (on)...");
        mount = MountState::BRAVIARY_ON;
    }
#if 0
    if (mount == MountState::NOTHING){
        if (m_last_good_state + std::chrono::seconds(2) < timestamp){
            return run_state_action(State::UNKNOWN);
        }
        return false;
    }else{
        m_last_good_state = timestamp;
    }
#endif

    switch (mount){
    case MountState::NOTHING:
        return false;
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
    //  Need to get on Sneasler.
    if (m_centerA.detected() && timestamp - last_state_change() > std::chrono::seconds(2)){
        return run_state_action(State::GET_ON_SNEASLER);
    }

//    cout << "m_last_known_flag_y = " << m_last_known_flag_y << endl;

    State state = (State)this->last_state();
    if (m_last_known_flag_y > 0.9 && timestamp - last_state_change() > std::chrono::seconds(2)){
//        cout << "state = " << (size_t)state << endl;
        switch (state){
        case State::DASH_FORWARD_MASH_B:
        case State::DASH_FORWARD_HOLD_B:
        case State::DIVE:
            m_console.log("Target passed under you. Target reached.");
            return true;
        default:;
        }
    }

    //  Find the flag.
    if (!m_flag_detected){
        return run_state_action(State::FIND_FLAG);
    }

    //  Dive
    if (m_flag_y > 0.50){
        return run_state_action(State::DIVE);
    }

    //  Re-center the flag.
    if (m_flag_x <= 0.4){
        return run_state_action(State::TURN_LEFT);
    }
    if (m_flag_x >= 0.6){
        return run_state_action(State::TURN_RIGHT);
    }

    //  Continue dive
    if (state == State::DIVE && m_flag_y > 0.20){
        return false;
    }

    //  Centered
    if (0.45 <= m_flag_x && m_flag_x <= 0.55){
        //  Cruise
        if (state != State::DIVE && m_flag_y <= 0.25){
            return run_state_action(State::DASH_FORWARD_MASH_B);
        }
        if (m_flag_y <= 0.50){
            return run_state_action(State::DASH_FORWARD_HOLD_B);
        }
    }

    //  Turning Cruise
    if (0.40 <= m_flag_x && m_flag_x <= 0.45 && m_flag_y <= 0.6){
        return run_state_action(State::DASH_LEFT);
    }
    if (0.55 <= m_flag_x && m_flag_x <= 0.60 && m_flag_y <= 0.6){
        return run_state_action(State::DASH_RIGHT);
    }

    //  No known state left.
    return run_state_action(State::DIVE);
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
