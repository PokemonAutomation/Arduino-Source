/*  Escape From Attack
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_EscapeFromAttack_H
#define PokemonAutomation_PokemonLA_EscapeFromAttack_H

#include "CommonFramework/Tools/SuperControlSession.h"
#include "PokemonLA/Inference/PokemonLA_UnderAttackDetector.h"
#include "PokemonLA/Inference/PokemonLA_MountDetector.h"
#include "PokemonLA/Inference/Objects/PokemonLA_ButtonDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


//  TODO: Add shiny sound detector.

class EscapeFromAttack : public SuperControlSession{
public:
    EscapeFromAttack(
        ProgramEnvironment& env,
        ConsoleHandle& console,
        std::chrono::seconds time_min,
        std::chrono::seconds time_limit
    );

    UnderAttackState state() const{
        return m_attacked.state();
    }

private:
    virtual bool run_state(AsyncCommandSession& commands) override;

    using WallClock = std::chrono::system_clock::time_point;
    bool run_flying(AsyncCommandSession& commands, WallClock timestamp);
    bool run_climbing(AsyncCommandSession& commands, WallClock timestamp);

private:
    static const uint16_t GET_ON_MOUNT_TIME = 125;
    static const uint16_t GET_ON_BRAVIARY_TIME = 280;

    enum class CurrentAction{
        OTHER,
        MOVE,
        CRUISE,
        CLIMBING,
    };

    std::chrono::system_clock::time_point m_min_stop;
    std::chrono::system_clock::time_point m_deadline;

    UnderAttackWatcher m_attacked;
    MountTracker m_mount;
    ButtonDetector m_centerA;
    ButtonDetector m_leftB;

    CurrentAction m_current_action;
};



}
}
}
#endif
