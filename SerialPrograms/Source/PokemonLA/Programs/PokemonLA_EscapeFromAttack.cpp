/*  Escape From Attack
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Tools/VideoStream.h"
#include "CommonTools/Async/InterruptableCommands.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonLA_EscapeFromAttack.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{



EscapeFromAttack::EscapeFromAttack(
    ProgramEnvironment& env,
    VideoStream& stream, ProControllerContext& context,
    std::chrono::seconds time_min,
    std::chrono::seconds time_limit
)
    : SuperControlSession(env, stream, context)
    , m_min_stop(current_time() + time_min)
    , m_deadline(current_time() + time_limit)
    , m_attacked(stream.logger())
    , m_mount(stream.logger())
    , m_centerA(
        stream.logger(), stream.overlay(),
        ButtonType::ButtonA,
        {0.40, 0.50, 0.40, 0.50},
        std::chrono::milliseconds(200),
        false
    )
    , m_leftB(
        stream.logger(), stream.overlay(),
        ButtonType::ButtonB,
        {0.02, 0.40, 0.05, 0.20},
        std::chrono::milliseconds(200),
        false
    )
    , m_get_on_sneasler_time(WallClock::min())
{
    *this += m_attacked;
    *this += m_mount;
    *this += m_centerA;
    *this += m_leftB;

    const std::chrono::milliseconds GET_ON_BRAVIARY_TIME_MILLIS(GET_ON_BRAVIARY_TIME * 1000 / TICKS_PER_SECOND);

    register_state_command(State::UNKNOWN, [this](){
        m_stream.log("Unknown state. Moving forward...");
        m_active_command->dispatch([](ProControllerContext& context){
            pbf_move_left_joystick(context, 128, 0, 300 * TICKS_PER_SECOND, 0);
        });
        return false;
    });
    register_state_command(State::WYRDEER_BASCULEGION_OFF, [this](){
        m_stream.log("Switching from Wyrdeer/Basculegion (off) to Braviary (on)...");
        m_active_command->dispatch([](ProControllerContext& context){
            pbf_press_dpad(context, DPAD_RIGHT, 20, 50);
            pbf_press_button(context, BUTTON_PLUS, 20, GET_ON_BRAVIARY_TIME);
        });
        return false;
    });
    register_state_command(State::WYRDEER_BASCULEGION_ON, [this](){
        m_stream.log("Switching from Wyrdeer/Basculegion (on) to Braviary (on)...");
        m_active_command->dispatch([](ProControllerContext& context){
            pbf_press_dpad(context, DPAD_RIGHT, 20, GET_ON_BRAVIARY_TIME);
        });
        return false;
    });
    register_state_command(State::URSALUNA_OFF, [this](){
        m_stream.log("Switching from Ursaluna (off) to Braviary (on)...");
        m_active_command->dispatch([](ProControllerContext& context){
            pbf_press_dpad(context, DPAD_RIGHT, 20, 50);
            pbf_press_dpad(context, DPAD_RIGHT, 20, 50);
            pbf_press_button(context, BUTTON_PLUS, 20, GET_ON_BRAVIARY_TIME);
        });
        return false;
    });
    register_state_command(State::URSALUNA_ON, [this](){
        m_stream.log("Switching from Ursaluna (on) to Braviary (on)...");
        m_active_command->dispatch([](ProControllerContext& context){
            pbf_press_dpad(context, DPAD_RIGHT, 20, 50);
            pbf_press_dpad(context, DPAD_RIGHT, 20, GET_ON_BRAVIARY_TIME);
        });
        return false;
    });
    register_state_command(State::SNEASLER_OFF, [this](){
        m_stream.log("Switching from Sneasler (off) to Braviary (on)...");
        m_active_command->dispatch([](ProControllerContext& context){
            pbf_press_dpad(context, DPAD_LEFT, 20, 50);
            pbf_press_button(context, BUTTON_PLUS, 20, GET_ON_BRAVIARY_TIME);
        });
        return false;
    });
    register_state_command(State::SNEASLER_ON, [this](){
        m_stream.log("Switching from Sneasler (on) to Braviary (on)...");
        m_active_command->dispatch([](ProControllerContext& context){
            pbf_move_left_joystick(context, 128, 0, 125, 0);
            pbf_press_dpad(context, DPAD_LEFT, 20, GET_ON_BRAVIARY_TIME);
        });
        return false;
    });
    register_state_command(State::BRAVIARY_OFF, [this](){
        m_stream.log("Switching from Braviary (off) to Braviary (on)...");
        m_active_command->dispatch([](ProControllerContext& context){
            pbf_press_button(context, BUTTON_PLUS, 20, GET_ON_BRAVIARY_TIME);
        });
        return false;
    });
    register_state_command(State::DASH_FORWARD, [this, GET_ON_BRAVIARY_TIME_MILLIS](){
        bool delay_dash =
            current_time() < m_get_on_sneasler_time + GET_ON_BRAVIARY_TIME_MILLIS;
        if (delay_dash){
            m_stream.log("Dashing forward... (delayed due to being on Sneasler)");
        }else{
            m_stream.log("Dashing forward...");
        }
        m_active_command->dispatch([delay_dash](ProControllerContext& context){
            if (delay_dash){
                pbf_move_left_joystick(context, 128, 0, 125, 0);
            }
            pbf_mash_button(context, BUTTON_B, 300 * TICKS_PER_SECOND);
        });
        return false;
    });
    register_state_command(State::GET_ON_SNEASLER, [this](){
        m_stream.log("Getting on Sneasler...");
        m_active_command->dispatch([](ProControllerContext& context){
            pbf_press_button(context, BUTTON_A, 20, 230);
        });
        m_get_on_sneasler_time = current_time();
        return false;
    });
    register_state_command(State::CLIMBING, [this](){
        m_stream.log("Climbing wall...");
        m_active_command->dispatch([](ProControllerContext& context){
            pbf_move_left_joystick(context, 128, 0, 300 * TICKS_PER_SECOND, 0);
        });
        return false;
    });
}
bool EscapeFromAttack::run_state(AsyncCommandSession<ProController>& commands, WallClock timestamp){
    if (timestamp > m_deadline){
        return true;
    }
    if (m_attacked.state() == UnderAttackState::SAFE && timestamp >= m_min_stop){
        return true;
    }

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

    m_stream.log("No state handler for current state.", COLOR_RED);
    return false;
}
bool EscapeFromAttack::run_flying(AsyncCommandSession<ProController>& commands, WallClock timestamp){
    if (m_centerA.detected() && timestamp - last_state_change() > std::chrono::seconds(3)){
        return run_state_action(State::GET_ON_SNEASLER);
    }
    return run_state_action(State::DASH_FORWARD);
}
bool EscapeFromAttack::run_climbing(AsyncCommandSession<ProController>& commands, WallClock timestamp){
    //  Can't jump off means you're able to stand. Switch back to Braviary.
    if (!m_leftB.detected()){
        return run_state_action(State::SNEASLER_OFF);
    }
    return run_state_action(State::CLIMBING);
}


}
}
}
