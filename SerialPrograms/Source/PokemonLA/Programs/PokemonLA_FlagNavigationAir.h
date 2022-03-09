/*  Flag Navigation (Air)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_FlagNavigation_H
#define PokemonAutomation_PokemonLA_FlagNavigation_H

#include "CommonFramework/Tools/SuperControlSession.h"
#include "PokemonLA/Inference/Objects/PokemonLA_FlagTracker.h"
#include "PokemonLA/Inference/Objects/PokemonLA_ButtonDetector.h"
#include "PokemonLA/Inference/PokemonLA_MountDetector.h"
#include "PokemonLA/Inference/PokemonLA_ShinySoundDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class FlagNavigationAir : public SuperControlSession{
public:
    FlagNavigationAir(
        ProgramEnvironment& env, ConsoleHandle& console,
        bool stop_on_shiny
    );

    bool detected_shiny() const{
        return m_shiny_listener.detected();
    }
    ShinySoundResults shiny_sound_results(){
        return m_shiny_listener.results();
    }


private:
    virtual bool run_state(AsyncCommandSession& commands, WallClock timestamp) override;

    bool run_flying(AsyncCommandSession& commands, WallClock timestamp);
    bool run_climbing(AsyncCommandSession& commands, WallClock timestamp);


private:
    static const uint16_t GET_ON_MOUNT_TIME = 125;
    static const uint16_t GET_ON_BRAVIARY_TIME = 280;

    enum class State{
        UNKNOWN,
        WYRDEER_BASCULEGION_OFF,
        WYRDEER_BASCULEGION_ON,
        URSALUNA_OFF,
        URSALUNA_ON,
        SNEASLER_OFF,
        SNEASLER_ON,
        BRAVIARY_OFF,
        GET_ON_SNEASLER,
        CLIMBING,
        DASH_FORWARD_MASH_B,
        DASH_FORWARD_HOLD_B,
        DIVE,
        DASH_LEFT,
        DASH_RIGHT,
        TURN_LEFT,
        TURN_RIGHT,
        FIND_FLAG,
    };
    void register_state_command(State state, std::function<bool()>&& action){
        SuperControlSession::register_state_command((size_t)state, std::move(action));
    }
    bool run_state_action(State state){
        return SuperControlSession::run_state_action((size_t)state);
    }

    bool m_stop_on_shiny;

    FlagTracker m_flag;
    MountTracker m_mount;
    ButtonDetector m_centerA;
    ButtonDetector m_leftB;
    ShinySoundDetector m_shiny_listener;

    std::atomic<bool> m_looking_straight_ahead;
    WallClock m_last_good_state;

    std::atomic<bool> m_find_flag_failed;

    bool m_flag_detected;
    double m_flag_distance;
    double m_flag_x;
    double m_flag_y;
    double m_last_known_flag_y;
};



}
}
}
#endif
