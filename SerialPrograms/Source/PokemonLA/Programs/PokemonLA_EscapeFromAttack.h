/*  Escape From Attack
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_EscapeFromAttack_H
#define PokemonAutomation_PokemonLA_EscapeFromAttack_H

#include "CommonTools/Async/SuperControlSession.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_Controller.h"
#include "PokemonLA/Inference/PokemonLA_UnderAttackDetector.h"
#include "PokemonLA/Inference/PokemonLA_MountDetector.h"
#include "PokemonLA/Inference/Objects/PokemonLA_ButtonDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class EscapeFromAttack : public SuperControlSession<SwitchController>{
public:
    EscapeFromAttack(
        ProgramEnvironment& env,
        VideoStream& stream, SwitchControllerContext& context,
        std::chrono::seconds time_min,
        std::chrono::seconds time_limit
    );

    UnderAttackState state() const{
        return m_attacked.state();
    }


private:
    enum class State{
        UNKNOWN,
        WYRDEER_BASCULEGION_OFF,
        WYRDEER_BASCULEGION_ON,
        URSALUNA_OFF,
        URSALUNA_ON,
        SNEASLER_OFF,
        SNEASLER_ON,
        BRAVIARY_OFF,
        DASH_FORWARD,
        GET_ON_SNEASLER,
        CLIMBING,
    };
    void register_state_command(State state, std::function<bool()>&& action){
        SuperControlSession::register_state_command((size_t)state, std::move(action));
    }
    bool run_state_action(State state){
        return SuperControlSession::run_state_action((size_t)state);
    }

    virtual bool run_state(AsyncCommandSession<SwitchController>& commands, WallClock timestamp) override;

    bool run_flying(AsyncCommandSession<SwitchController>& commands, WallClock timestamp);
    bool run_climbing(AsyncCommandSession<SwitchController>& commands, WallClock timestamp);

private:
    static const uint16_t GET_ON_MOUNT_TIME = 125;
    static const uint16_t GET_ON_BRAVIARY_TIME = 280;


    const WallClock m_min_stop;
    const WallClock m_deadline;

    UnderAttackWatcher m_attacked;
    MountTracker m_mount;
    ButtonDetector m_centerA;
    ButtonDetector m_leftB;

    WallClock m_get_on_sneasler_time;
};



}
}
}
#endif
