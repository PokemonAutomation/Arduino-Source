/*  Flag Navigation (Air)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLA_FlagNavigation_H
#define PokemonAutomation_PokemonLA_FlagNavigation_H

#include "CommonTools/Async/SuperControlSession.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "PokemonLA/Inference/Objects/PokemonLA_FlagTracker.h"
#include "PokemonLA/Inference/Objects/PokemonLA_ButtonDetector.h"
#include "PokemonLA/Inference/PokemonLA_DialogDetector.h"
#include "PokemonLA/Inference/PokemonLA_MountDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class FlagNavigationAir : public SuperControlSession<ProController>{
public:
    FlagNavigationAir(
        ProgramEnvironment& env,
        VideoStream& stream, ProControllerContext& context,
        uint16_t stop_radius,
        double flag_reached_delay,
        std::chrono::seconds navigate_timeout
    );

    void set_distance_callback(std::function<void(double distance)> flag_callback);


private:
    virtual bool run_state(AsyncCommandSession<ProController>& commands, WallClock timestamp) override;

    bool run_flying(AsyncCommandSession<ProController>& commands, WallClock timestamp);
    bool run_climbing(AsyncCommandSession<ProController>& commands, WallClock timestamp);


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
        DASH_LEFT,
        DASH_RIGHT,
        DIVE_STRAIGHT,
        DIVE_LEFT,
        DIVE_RIGHT,
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

    uint16_t m_stop_radius;
    std::chrono::milliseconds m_flag_reached_delay;
    std::chrono::seconds m_navigate_timeout;

    std::function<void(double distance)> m_flag_callback;

    FlagTracker m_flag;
    MountTracker m_mount;
    ButtonDetector m_centerA;
    ButtonDetector m_leftB;
    DialogSurpriseDetector m_dialog_detector;

    std::atomic<bool> m_looking_straight_ahead;
    std::atomic<WallClock> m_looking_straight_ahead_timestamp;
//    WallClock m_last_good_state;
    MountState m_last_known_mount;

    std::atomic<bool> m_find_flag_failed;
    WallClock m_flag_reached_time;

    WallClock m_last_flag_detection;
    bool m_flag_detected;

    //  Last known values.
    double m_flag_distance;
    double m_flag_x;
    double m_flag_y;

    WallClock m_last_flag_print;
};



}
}
}
#endif
