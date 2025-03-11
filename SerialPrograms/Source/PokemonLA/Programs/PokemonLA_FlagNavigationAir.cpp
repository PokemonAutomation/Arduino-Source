/*  Flag Navigation (Air)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <sstream>
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonTools/Async/InterruptableCommands.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonLA_FlagNavigationAir.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


FlagNavigationAir::FlagNavigationAir(
    ProgramEnvironment& env, VideoStream& stream, ProControllerContext& context,
    uint16_t stop_radius,
    double flag_reached_delay,
    std::chrono::seconds navigate_timeout
)
    : SuperControlSession(env, stream, context)
    , m_stop_radius(stop_radius)
    , m_flag_reached_delay(std::chrono::milliseconds((uint64_t)(flag_reached_delay * 1000)))
    , m_navigate_timeout(navigate_timeout)
    , m_flag(stream.logger(), stream.overlay())
    , m_mount(stream.logger())
    , m_centerA(stream.logger(), stream.overlay(), ButtonType::ButtonA, {0.40, 0.50, 0.40, 0.50}, std::chrono::milliseconds(200), false)
    , m_leftB(stream.logger(), stream.overlay(), ButtonType::ButtonB, {0.02, 0.40, 0.05, 0.20}, std::chrono::milliseconds(200), false)
    , m_dialog_detector(stream.logger(), stream.overlay(), false)
    , m_looking_straight_ahead(false)
    , m_looking_straight_ahead_timestamp(WallClock::min())
//    , m_last_good_state(WallClock::min())
    , m_last_known_mount(MountState::NOTHING)
    , m_find_flag_failed(false)
    , m_flag_reached_time(WallClock::max())
    , m_last_flag_detection(WallClock::min())
    , m_flag_distance(-1)
    , m_flag_x(0.5)
    , m_flag_y(0.0)
    , m_last_flag_print(WallClock::min())
{
    *this += m_flag;
    *this += m_mount;
    *this += m_centerA;
    *this += m_leftB;
    *this += m_dialog_detector;

    auto find_flag = [this](ProControllerContext& context){
        uint8_t turn = m_flag_x <= 0.5 ? 0 : 255;
        for (size_t c = 0; c < 2; c++){
            pbf_mash_button(context, BUTTON_ZL, 2 * TICKS_PER_SECOND);
            pbf_move_right_joystick(context, turn, 128, 400, 0);
            pbf_move_right_joystick(context, 128, 255, 120, 0);
            pbf_move_right_joystick(context, turn, 128, 400, 0);
            pbf_move_right_joystick(context, 128, 0, 200, 0);
            pbf_move_right_joystick(context, turn, 128, 400, 0);
        }
        context.wait_for_all_requests();
        m_find_flag_failed.store(true, std::memory_order_release);
    };

    register_state_command(State::UNKNOWN, [this, find_flag](){
        m_stream.log("Unknown state. Moving camera around...");
        m_active_command->dispatch(find_flag);
        m_looking_straight_ahead.store(false, std::memory_order_release);
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
//            pbf_press_dpad(context, DPAD_RIGHT, 20, 50);
//            pbf_press_button(context, BUTTON_PLUS, 20, GET_ON_BRAVIARY_TIME);
        });
        return false;
    });
    register_state_command(State::URSALUNA_ON, [this](){
        m_stream.log("Switching from Ursaluna (on) to Braviary (on)...");
        m_active_command->dispatch([](ProControllerContext& context){
            pbf_press_dpad(context, DPAD_RIGHT, 20, 50);
//            pbf_press_dpad(context, DPAD_RIGHT, 20, GET_ON_BRAVIARY_TIME);
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
        m_looking_straight_ahead.store(false, std::memory_order_release);
        m_active_command->dispatch([](ProControllerContext& context){
            pbf_move_left_joystick(context, 128, 0, 125, 0);
            pbf_press_dpad(context, DPAD_LEFT, 20, GET_ON_BRAVIARY_TIME);
        });
        return false;
    });
    register_state_command(State::BRAVIARY_OFF, [this](){
        m_stream.log("Switching from Braviary (off) to Braviary (on)...");
        m_looking_straight_ahead.store(false, std::memory_order_release);
        m_active_command->dispatch([](ProControllerContext& context){
            pbf_press_button(context, BUTTON_PLUS, 20, GET_ON_BRAVIARY_TIME);
        });
        return false;
    });
    register_state_command(State::GET_ON_SNEASLER, [this](){
        m_stream.log("Getting on Sneasler...");
        m_looking_straight_ahead.store(false, std::memory_order_release);
        m_active_command->dispatch([](ProControllerContext& context){
            pbf_press_button(context, BUTTON_A, 20, 230);
        });
        return false;
    });
    register_state_command(State::CLIMBING, [this](){
        m_stream.log("Climbing wall...");
        m_looking_straight_ahead.store(false, std::memory_order_release);
        m_active_command->dispatch([](ProControllerContext& context){
            pbf_move_left_joystick(context, 128, 0, 300 * TICKS_PER_SECOND, 0);
        });
        return false;
    });

    register_state_command(State::DASH_FORWARD_MASH_B, [this](){
        m_stream.log("Dashing forward (mash B)...");
        m_active_command->dispatch([this](ProControllerContext& context){
            //  Move forward to straighten out direction.
            if (!m_looking_straight_ahead.load(std::memory_order_acquire)){
                pbf_move_left_joystick(context, 128, 0, 160, 0);
                context.wait_for_all_requests();
                m_looking_straight_ahead_timestamp.store(current_time());
                m_looking_straight_ahead.store(true, std::memory_order_release);
//                cout << "State::DASH_FORWARD: m_looking_straight_ahead = true" << endl;
            }
            pbf_mash_button(context, BUTTON_B, 300 * TICKS_PER_SECOND);
        });
        return false;
    });
    register_state_command(State::DASH_FORWARD_HOLD_B, [this](){
        m_stream.log("Dashing forward (hold B)...");
        m_active_command->dispatch([this](ProControllerContext& context){
            //  Move forward to straighten out direction.
            if (!m_looking_straight_ahead.load(std::memory_order_acquire)){
                pbf_move_left_joystick(context, 128, 0, 160, 0);
                context.wait_for_all_requests();
                m_looking_straight_ahead_timestamp.store(current_time());
                m_looking_straight_ahead.store(true, std::memory_order_release);
//                cout << "State::DASH_FORWARD: m_looking_straight_ahead = true" << endl;
            }
            pbf_press_button(context, BUTTON_B, 300 * TICKS_PER_SECOND, 0);
        });
        return false;
    });

    auto dash_turn = [this](){
        m_stream.log("Dashing Turn...");
        m_active_command->dispatch([this](ProControllerContext& context){
            //  Move forward to straighten out direction.
//            cout << "Straight ahead = " << m_looking_straight_ahead.load(std::memory_order_acquire) << endl;
            if (!m_looking_straight_ahead.load(std::memory_order_acquire)){
//                pbf_wait(context, 1000ms);
                pbf_move_left_joystick(context, 128, 0, 160, 0);
                context.wait_for_all_requests();
                m_looking_straight_ahead_timestamp.store(current_time());
                m_looking_straight_ahead.store(true, std::memory_order_release);
//                cout << "State::DASH_LEFT: m_looking_straight_ahead = true" << endl;
            }
//            pbf_press_button(context, BUTTON_B, 10, 0);
            double shift = 0;
            double distance, flag_x, flag_y;
            if (m_flag.get(distance, flag_x, flag_y)){
                shift = (flag_x - 0.5) * 320;
                shift = std::max(shift, -32.);
                shift = std::min(shift, 32.);
            }
            pbf_controller_state(context, BUTTON_B, DPAD_NONE, (int8_t)(128 + shift), 128, 128, 128, 255);
        });
        return false;
    };
    register_state_command(State::DASH_LEFT, dash_turn);
    register_state_command(State::DASH_RIGHT, dash_turn);

    register_state_command(State::DIVE_STRAIGHT, [this](){
        m_stream.log("Diving Straight...");
        m_active_command->dispatch([this](ProControllerContext& context){
            //  Move forward to straighten out direction.
            if (!m_looking_straight_ahead.load(std::memory_order_acquire)){
                pbf_move_left_joystick(context, 128, 0, 160, 0);
                context.wait_for_all_requests();
                m_looking_straight_ahead_timestamp.store(current_time());
                m_looking_straight_ahead.store(true, std::memory_order_release);
//                cout << "State::DASH_FORWARD: m_looking_straight_ahead = true" << endl;
            }
            pbf_press_button(context, BUTTON_Y, 60 * TICKS_PER_SECOND, 0);
        });
        return false;
    });

    auto dive_turn = [this](){
        m_stream.log("Diving Turn...");
        m_active_command->dispatch([this](ProControllerContext& context){
            //  Move forward to straighten out direction.
            if (!m_looking_straight_ahead.load(std::memory_order_acquire)){
                pbf_move_left_joystick(context, 128, 0, 160, 0);
                context.wait_for_all_requests();
                m_looking_straight_ahead_timestamp.store(current_time());
                m_looking_straight_ahead.store(true, std::memory_order_release);
//                cout << "State::DASH_LEFT: m_looking_straight_ahead = true" << endl;
            }
//            pbf_press_button(context, BUTTON_Y, 10, 0);
            double shift = 0;
            double distance, flag_x, flag_y;
            if (m_flag.get(distance, flag_x, flag_y)){
                shift = (flag_x - 0.5) * 640;
                shift = std::max(shift, -32.);
                shift = std::min(shift, 32.);
            }
            pbf_controller_state(context, BUTTON_Y, DPAD_NONE, (int8_t)(128 + shift), 128, 128, 128, 125);
        });
        return false;
    };
    register_state_command(State::DIVE_LEFT, dive_turn);
    register_state_command(State::DIVE_RIGHT, dive_turn);

    register_state_command(State::TURN_LEFT, [this](){
        m_stream.log("Turning Left...");
        m_active_command->dispatch([this](ProControllerContext& context){
            pbf_wait(context, 150);
            context.wait_for_all_requests();
            double distance, flag_x, flag_y;
            if (m_flag.get(distance, flag_x, flag_y)){
                pbf_move_right_joystick(context, 0, 128, (uint16_t)(85 * (0.5 - flag_x)), 0);
            }
        });
        m_looking_straight_ahead.store(false, std::memory_order_release);
        return false;
    });
    register_state_command(State::TURN_RIGHT, [this](){
        m_stream.log("Turning Right...");
        m_active_command->dispatch([this](ProControllerContext& context){
            pbf_wait(context, 150);
            context.wait_for_all_requests();
            double distance, flag_x, flag_y;
            if (m_flag.get(distance, flag_x, flag_y)){
                pbf_move_right_joystick(context, 255, 128, (uint16_t)(85 * (flag_x - 0.5)), 0);
            }
        });
        m_looking_straight_ahead.store(false, std::memory_order_release);
        return false;
    });

    register_state_command(State::FIND_FLAG, [this, find_flag](){
        m_stream.log("Looking for flag...");
        m_active_command->dispatch(find_flag);
        m_looking_straight_ahead.store(false, std::memory_order_release);
        return false;
    });
}
void FlagNavigationAir::set_distance_callback(std::function<void(double distance)> flag_callback){
    m_flag_callback = std::move(flag_callback);
}

bool FlagNavigationAir::run_state(AsyncCommandSession<ProController>& commands, WallClock timestamp){
    if (last_state_change() + std::chrono::seconds(60) < timestamp){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "No state change detected after 60 seconds.",
            m_stream
        );
    }
    if (start_time() + m_navigate_timeout < timestamp){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Unable to reach flag after timeout period.",
            m_stream
        );
    }
    if (m_dialog_detector.detected()){
        OperationFailedException::fire(
            ErrorReport::NO_ERROR_REPORT,
            "Potential ambush by Miss Fortune sister.",
            m_stream
        );
    }
    if (m_find_flag_failed.load(std::memory_order_acquire)){
        OperationFailedException::fire(
            ErrorReport::NO_ERROR_REPORT,
            "Unable to find flag.",
            m_stream
        );
    }

    //  Read flag.
    m_flag_detected = m_flag.get(m_flag_distance, m_flag_x, m_flag_y, timestamp);
    if (m_flag_detected){
        m_last_flag_detection = timestamp;
        if (m_flag_callback){
            m_flag_callback(m_flag_distance);
        }
    }else{
//        m_console.log("Flag not detected.", COLOR_ORANGE);
    }
#if 1
    if (true
        && m_flag_detected
        && m_flag_distance >= 0
        && m_last_flag_print + std::chrono::seconds(1) <= timestamp
    ){
        std::ostringstream ss;
        ss << "Flag: Distance = " << (m_flag_distance < 0 ? "?" : std::to_string(m_flag_distance))
           << ", x = " << m_flag_x << ", y = " << m_flag_y << std::endl;
        m_stream.log(ss.str(), COLOR_PURPLE);
        m_last_flag_print = timestamp;
    }
#endif


    //  Check if we've reached the flag.
    if (m_flag_distance >= 0){
        if (m_flag_distance > m_stop_radius){
            //  If we haven't reached the flag, reset the timer.
            m_flag_reached_time = WallClock::max();
        }else if (m_flag_distance <= m_stop_radius && m_flag_reached_time == WallClock::max()){
            //  If we've reached the flag, start timer if we haven't already.
            m_stream.log("Target reached. Waiting out grace period...");
            m_flag_reached_time = timestamp;
        }
    }
    if (m_flag_reached_time <= timestamp - m_flag_reached_delay){
        m_stream.log("Grace period finished. Stopping flag navigation...");
        return true;
    }


    MountState mount = m_mount.state();
    if (mount != MountState::NOTHING){
        m_last_known_mount = mount;
    }else{
        m_stream.log(
            std::string("Unable to detect mount. Assuming last known mount: ") +
            MOUNT_STATE_STRINGS[(size_t)m_last_known_mount]
        );
        mount = m_last_known_mount;
    }
    if (mount == MountState::NOTHING){
        return run_state_action(State::UNKNOWN);
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

    m_stream.log("No state handler for current state.", COLOR_RED);
    return false;
}
bool FlagNavigationAir::run_flying(AsyncCommandSession<ProController>& commands, WallClock timestamp){
    //  Need to get on Sneasler.
    if (m_centerA.detected() && last_state_change() + std::chrono::seconds(2) < timestamp){
        return run_state_action(State::GET_ON_SNEASLER);
    }

//    cout << "m_last_known_flag_y = " << m_last_known_flag_y << endl;

    State state = (State)this->last_state();
#if 0
    if (m_flag_y > 0.9 && last_state_change() + std::chrono::seconds(2) < timestamp){
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
#endif

//    if (m_last_flag_detection + std::chrono::seconds(20) < timestamp){
//        OperationFailedException::fire(m_console, "Flag not detected after 20 seconds.", true);
//    }
//    double flag_age = std::chrono::duration_cast<std::chrono::milliseconds>(timestamp - m_last_flag_detection).count() / 1000.;
//    if (flag_age > 0){
//        m_console.log("Flag Age = " + std::to_string(flag_age));
//    }

    //  Flag is stale.
    if (m_last_flag_detection + std::chrono::seconds(1) < timestamp){
        return run_state_action(State::FIND_FLAG);
    }


    //  Re-center the flag.
    if (m_flag_x <= 0.30){
        return run_state_action(State::TURN_LEFT);
    }
    if (m_flag_x >= 0.70){
        return run_state_action(State::TURN_RIGHT);
    }

    //  Dive
    double dive_threshold = 0.80;
    switch (state){
    case State::DIVE_STRAIGHT:
    case State::DIVE_LEFT:
    case State::DIVE_RIGHT:
        dive_threshold = 0.10;
        break;
    case State::DASH_FORWARD_HOLD_B:
    case State::DASH_FORWARD_MASH_B:
    case State::DASH_LEFT:
    case State::DASH_RIGHT:{
        if (m_looking_straight_ahead.load(std::memory_order_acquire) &&
            m_looking_straight_ahead_timestamp.load(std::memory_order_acquire) + std::chrono::seconds(1) < timestamp){
            dive_threshold = 0.45;
        }
        break;
    }
    default:;
    }
//    cout << "m_flag_y = " << m_flag_y << endl;
    if (m_flag_detected && m_flag_y > dive_threshold){
        if (0.49 <= m_flag_x && m_flag_x <= 0.51){
            return run_state_action(State::DIVE_STRAIGHT);
        }else{
            return run_state_action(m_flag_x < 0.5 ? State::DIVE_LEFT : State::DIVE_RIGHT);
        }
    }

    //  Turning Cruise
    if (0.48 > m_flag_x || m_flag_x > 0.52){
        return run_state_action(m_flag_x < 0.5 ? State::DASH_LEFT : State::DASH_RIGHT);
    }

    //  B-mash Cruise
    return run_state_action(State::DASH_FORWARD_MASH_B);

#if 0
    if (m_flag_y <= 0.40){
        //  B-mash Cruise
        return run_state_action(State::DASH_FORWARD_MASH_B);
    }else{
        //  Normal Cruise
        return run_state_action(State::DASH_FORWARD_HOLD_B);
    }

    //  No known state left.
    return run_state_action(State::DIVE_STRAIGHT);
#endif
}
bool FlagNavigationAir::run_climbing(AsyncCommandSession<ProController>& commands, WallClock timestamp){
    //  Can't jump off means you're able to stand. Switch back to Braviary.
    if (!m_leftB.detected()){
        return run_state_action(State::SNEASLER_ON);
    }
    return run_state_action(State::CLIMBING);
}






}
}
}
