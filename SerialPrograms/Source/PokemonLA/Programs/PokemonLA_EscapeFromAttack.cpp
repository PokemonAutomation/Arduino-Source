/*  Escape From Attack
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Tools/ConsoleHandle.h"
#include "CommonFramework/Tools/InterruptableCommands.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonLA_EscapeFromAttack.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{



EscapeFromAttack::EscapeFromAttack(
    ProgramEnvironment& env,
    ConsoleHandle& console,
    std::chrono::seconds time_min,
    std::chrono::seconds time_limit
)
    : SuperControlSession(env, console)
    , m_min_stop(std::chrono::system_clock::now() + time_min)
    , m_deadline(std::chrono::system_clock::now() + time_limit)
    , m_attacked(console)
    , m_mount(console)
    , m_centerA(console, console, ButtonType::ButtonA, {0.40, 0.50, 0.40, 0.50}, std::chrono::milliseconds(200), false)
    , m_leftB(console, console, ButtonType::ButtonB, {0.02, 0.40, 0.05, 0.20}, std::chrono::milliseconds(200), false)
    , m_current_action(CurrentAction::OTHER)
{
    m_visual_callbacks.emplace_back(&m_attacked);
    m_visual_callbacks.emplace_back(&m_mount);
    m_visual_callbacks.emplace_back(&m_centerA);
    m_visual_callbacks.emplace_back(&m_leftB);
}
bool EscapeFromAttack::run_state(AsyncCommandSession& commands){
    WallClock now = std::chrono::system_clock::now();
    if (now > m_deadline){
        return true;
    }

    if (m_attacked.state() == UnderAttackState::SAFE && now >= m_min_stop){
        return true;
    }

    switch (m_mount.state()){
    case MountState::NOTHING:
        if (m_current_action != CurrentAction::MOVE){
            m_console.log("Unable to detect mount. Moving foward...");
            commands.dispatch([=](const BotBaseContext& context){
                pbf_move_left_joystick(context, 128, 0, 300 * TICKS_PER_SECOND, 0);
            });
            m_current_action = CurrentAction::MOVE;
        }
        break;
    case MountState::WYRDEER_OFF:
    case MountState::BASCULEGION_OFF:
        commands.dispatch([=](const BotBaseContext& context){
            pbf_press_button(context, BUTTON_PLUS, 20, GET_ON_MOUNT_TIME);
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
bool EscapeFromAttack::run_flying(AsyncCommandSession& commands, WallClock timestamp){
    //  Need to get on Sneasler.
    if (m_centerA.detected()){
        m_console.log("Pressing A...");
        commands.dispatch([=](const BotBaseContext& context){
            pbf_press_button(context, BUTTON_A, 20, 230);
        });
        commands.wait();
        return false;
    }

    if (m_current_action != CurrentAction::CRUISE){
        m_console.log("Dash forward...");
        commands.dispatch([=](const BotBaseContext& context){
//            pbf_press_button(context, BUTTON_B, 300 * TICKS_PER_SECOND, 0);
            pbf_mash_button(context, BUTTON_B, 300 * TICKS_PER_SECOND);
        });
        m_current_action = CurrentAction::CRUISE;
    }

    m_env.wait_for(std::chrono::milliseconds(100));
    return false;
}
bool EscapeFromAttack::run_climbing(AsyncCommandSession& commands, WallClock timestamp){
    //  Can't jump off means you're able to stand. Switch back to Braviary.
    if (!m_leftB.detected()){
        m_console.log("Switching back to Braviary...");
        commands.dispatch([=](const BotBaseContext& context){
            pbf_move_left_joystick(context, 128, 0, 125, 0);
            pbf_press_dpad(context, DPAD_LEFT, 20, GET_ON_BRAVIARY_TIME);
        });
        commands.wait();
        return false;
    }

    if (m_current_action != CurrentAction::CLIMBING){
        m_console.log("Climbing wall...");
        commands.dispatch([=](const BotBaseContext& context){
            pbf_move_left_joystick(context, 128, 0, 300 * TICKS_PER_SECOND, 0);
        });
        m_current_action = CurrentAction::CLIMBING;
        return false;
    }

    return false;
}


}
}
}
