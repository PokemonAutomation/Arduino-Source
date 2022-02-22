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
    , m_centerA(console, console, ButtonType::ButtonA, {0.40, 0.50, 0.40, 0.50}, std::chrono::milliseconds(500), false)
    , m_leftB(console, console, ButtonType::ButtonB, {0.02, 0.40, 0.05, 0.20}, std::chrono::milliseconds(500), false)
    , m_current_action(CurrentAction::OTHER)
    , m_looking_straight_ahead(false)
    , m_last_turn(WallClock::min())
    , m_last_flag_find(WallClock::min())
{
    m_visual_callbacks.emplace_back(&m_flag);
    m_visual_callbacks.emplace_back(&m_mount);
    m_visual_callbacks.emplace_back(&m_centerA);
    m_visual_callbacks.emplace_back(&m_leftB);
}

bool FlagNavigationAir::run_state(AsyncCommandSession& commands){
    WallClock now = std::chrono::system_clock::now();

    switch (m_mount.state()){
    case MountState::NOTHING:
        break;
    case MountState::WYRDEER_OFF:
    case MountState::BASCULEGION_OFF:
        commands.dispatch([=](const BotBaseContext& context){
            pbf_press_button(context, BUTTON_PLUS, 20, GET_ON_BRAVIARY_TIME);
        });
        commands.wait();
        m_current_action = CurrentAction::OTHER;
        return false;
    case MountState::WYRDEER_ON:
    case MountState::BASCULEGION_ON:
        commands.dispatch([=](const BotBaseContext& context){
            pbf_press_dpad(context, DPAD_RIGHT, 20, GET_ON_BRAVIARY_TIME);
        });
        commands.wait();
        m_current_action = CurrentAction::OTHER;
        return false;
    case MountState::URSALUNA_OFF:
        commands.dispatch([=](const BotBaseContext& context){
            pbf_press_dpad(context, DPAD_RIGHT, 20, 50);
            pbf_press_dpad(context, DPAD_RIGHT, 20, 50);
            pbf_press_button(context, BUTTON_PLUS, 20, GET_ON_BRAVIARY_TIME);
        });
        commands.wait();
        m_current_action = CurrentAction::OTHER;
        return false;
    case MountState::URSALUNA_ON:
        commands.dispatch([=](const BotBaseContext& context){
            pbf_press_dpad(context, DPAD_RIGHT, 20, 50);
            pbf_press_dpad(context, DPAD_RIGHT, 20, GET_ON_BRAVIARY_TIME);
        });
        commands.wait();
        m_current_action = CurrentAction::OTHER;
        return false;
    case MountState::SNEASLER_OFF:
        commands.dispatch([=](const BotBaseContext& context){
            pbf_press_dpad(context, DPAD_LEFT, 20, 50);
            pbf_press_button(context, BUTTON_PLUS, 20, GET_ON_BRAVIARY_TIME);
        });
        commands.wait();
        m_current_action = CurrentAction::OTHER;
        return false;
    case MountState::SNEASLER_ON:
        return run_climbing(commands, now);
    case MountState::BRAVIARY_OFF:
        commands.dispatch([=](const BotBaseContext& context){
            pbf_press_button(context, BUTTON_PLUS, 20, GET_ON_BRAVIARY_TIME);
        });
        commands.wait();
        m_current_action = CurrentAction::OTHER;
        return false;
    case MountState::BRAVIARY_ON:
        return run_flying(commands, now);
    }

    m_console.log("No state handler for current state. Waiting 1 second.");
    m_env.wait_for(std::chrono::milliseconds(1000));

    return false;
}
bool FlagNavigationAir::run_flying(AsyncCommandSession& commands, WallClock timestamp){
    double flag_distance, flag_x, flag_y;
    bool flag_ok = m_flag.get(flag_distance, flag_x, flag_y);
//    cout << "flag_ok = " << flag_ok << ", x = " << flag_x << ", y = " << flag_y << endl;

    //  Need to get on Sneasler.
    if (m_centerA.detected()){
        m_console.log("Pressing A...");
        commands.dispatch([=](const BotBaseContext& context){
            pbf_press_button(context, BUTTON_A, 20, 230);
        });
        commands.wait();
        m_looking_straight_ahead = false;
        return false;
    }

    //  Cruise
    if (flag_ok && flag_y <= 0.6 && 0.45 <= flag_x && flag_x <= 0.55){
//        cout << "cruise" << endl;
        if (m_current_action != CurrentAction::CRUISE){
            m_console.log("Crusing Straight...");
            bool looking_straight_ahead = m_looking_straight_ahead;
            commands.dispatch([=](const BotBaseContext& context){
                //  Move forward to straighten out direction.
                if (!looking_straight_ahead){
                    pbf_move_left_joystick(context, 128, 0, 125, 0);
                }
                pbf_press_button(context, BUTTON_B, 300 * TICKS_PER_SECOND, 0);
            });
            m_current_action = CurrentAction::CRUISE;
            m_looking_straight_ahead = true;
        }
        m_env.wait_for(std::chrono::milliseconds(100));
        return false;
    }

    //  Turning Cruise
    if (flag_ok && flag_y <= 0.6 && 0.40 <= flag_x && flag_x <= 0.45){
        if (m_current_action != CurrentAction::CRUISE_TURN){
            m_console.log("Crusing Left...");
            bool looking_straight_ahead = m_looking_straight_ahead;
            commands.dispatch([=](const BotBaseContext& context){
                //  Move forward to straighten out direction.
                if (!looking_straight_ahead){
                    pbf_move_left_joystick(context, 128, 0, 125, 0);
                }
                pbf_press_button(context, BUTTON_B, 50, 0);
                pbf_controller_state(context, BUTTON_B, DPAD_NONE, 96, 128, 128, 128, 255);
            });
            m_current_action = CurrentAction::CRUISE_TURN;
            m_looking_straight_ahead = true;
        }
        m_env.wait_for(std::chrono::milliseconds(100));
        return false;
    }
    if (flag_ok && flag_y <= 0.6 && 0.55 <= flag_x && flag_x <= 0.60){
        if (m_current_action != CurrentAction::CRUISE_TURN){
            m_console.log("Cruising Right...");
            bool looking_straight_ahead = m_looking_straight_ahead;
            commands.dispatch([=](const BotBaseContext& context){
                //  Move forward to straighten out direction.
                if (!looking_straight_ahead){
                    pbf_move_left_joystick(context, 128, 0, 125, 0);
                }
                pbf_press_button(context, BUTTON_B, 50, 0);
                pbf_controller_state(context, BUTTON_B, DPAD_NONE, 160, 128, 128, 128, 255);
            });
            m_current_action = CurrentAction::CRUISE_TURN;
            m_looking_straight_ahead = true;
        }
        m_env.wait_for(std::chrono::milliseconds(100));
        return false;
    }

    //  Re-center the flag.
    if (flag_ok && flag_x <= 0.4 && timestamp > m_last_turn + std::chrono::seconds(2)){
        m_console.log("Turning Left...");
        m_current_action = CurrentAction::CAMERA_TURN;
        m_last_turn = timestamp;
        commands.dispatch([=](const BotBaseContext& context){
            pbf_wait(context, 125);
            pbf_move_right_joystick(context, 0, 128, (uint16_t)(85 * (0.5 - flag_x)), 0);
            pbf_move_left_joystick(context, 128, 0, 125, 0);
        });
        commands.wait();
        m_looking_straight_ahead = false;
        return false;
    }
    if (flag_ok && flag_x >= 0.6 && timestamp > m_last_turn + std::chrono::seconds(2)){
        m_console.log("Turning Right...");
        m_current_action = CurrentAction::CAMERA_TURN;
        m_last_turn = timestamp;
        commands.dispatch([=](const BotBaseContext& context){
            pbf_wait(context, 125);
            pbf_move_right_joystick(context, 255, 128, (uint16_t)(85 * (flag_x - 0.5)), 0);
            pbf_move_left_joystick(context, 128, 0, 125, 0);
        });
        commands.wait();
        m_looking_straight_ahead = false;
        return false;
    }

    //  Find the flag.
    if (!flag_ok && m_current_action != CurrentAction::FIND_FLAG && timestamp > m_last_flag_find + std::chrono::seconds(5)){
        m_console.log("Looking for flag...");
        m_current_action = CurrentAction::FIND_FLAG;
        m_last_flag_find = timestamp;
        commands.dispatch([=](const BotBaseContext& context){
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
    }

    m_env.wait_for(std::chrono::milliseconds(100));
    return false;
}
bool FlagNavigationAir::run_climbing(AsyncCommandSession& commands, WallClock timestamp){
    //  Can't jump off means you're able to stand. Switch back to Braviary.
    if (!m_leftB.detected()){
        m_console.log("Switching back to Braviary...");
        commands.dispatch([=](const BotBaseContext& context){
            pbf_move_left_joystick(context, 128, 0, 125, 0);
            pbf_press_dpad(context, DPAD_LEFT, 20, GET_ON_BRAVIARY_TIME);
        });
        commands.wait();
        m_looking_straight_ahead = false;
        return false;
    }

    if (m_current_action != CurrentAction::CLIMBING){
        m_console.log("Climbing wall...");
        commands.dispatch([=](const BotBaseContext& context){
            pbf_move_left_joystick(context, 128, 0, 300 * TICKS_PER_SECOND, 0);
        });
        m_current_action = CurrentAction::CLIMBING;
        m_looking_straight_ahead = false;
        return false;
    }

    return false;
}



}
}
}
