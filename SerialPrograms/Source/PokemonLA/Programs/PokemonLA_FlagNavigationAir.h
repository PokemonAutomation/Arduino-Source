/*  Flag Navigation (Air)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_FlagNavigation_H
#define PokemonAutomation_PokemonLA_FlagNavigation_H

#include "CommonFramework/Tools/SuperControlSession.h"
#include "PokemonLA/Inference/PokemonLA_FlagTracker.h"
#include "PokemonLA/Inference/PokemonLA_MountDetector.h"
#include "PokemonLA/Inference/PokemonLA_ButtonDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class FlagNavigationAir : public SuperControlSession{
public:
    FlagNavigationAir(ProgramEnvironment& env, ConsoleHandle& console);


private:
    virtual bool run_state(AsyncCommandSession& commands) override;

    using WallClock = std::chrono::system_clock::time_point;
    bool run_flying(AsyncCommandSession& commands, WallClock timestamp);
    bool run_climbing(AsyncCommandSession& commands, WallClock timestamp);


private:
    enum class CurrentAction{
        OTHER,
        CRUISE,
        CRUISE_TURN,
        CAMERA_TURN,
        FIND_FLAG,
        CLIMBING,
    };

    FlagTracker m_flag;
    MountTracker m_mount;
    ButtonDetector m_centerA;
    ButtonDetector m_leftB;

    CurrentAction m_current_action;
    bool m_looking_straight_ahead;

    WallClock m_last_turn;
    WallClock m_last_flag_find;
};



}
}
}
#endif
