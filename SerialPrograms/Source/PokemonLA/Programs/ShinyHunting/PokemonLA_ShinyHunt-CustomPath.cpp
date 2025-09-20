/*  Shiny Hunt - Custom Path
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLA/PokemonLA_Settings.h"
#include "PokemonLA/Inference/Sounds/PokemonLA_ShinySoundDetector.h"
#include "PokemonLA/Programs/PokemonLA_GameEntry.h"
#include "PokemonLA/Programs/PokemonLA_RegionNavigation.h"
#include "PokemonLA/Programs/PokemonLA_MountChange.h"
#include "PokemonLA/Programs/PokemonLA_TimeOfDayChange.h"
#include "PokemonLA_ShinyHunt-CustomPath.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{
    using namespace Pokemon;


ShinyHuntCustomPath_Descriptor::ShinyHuntCustomPath_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLA:ShinyHunt-CustomPath",
        STRING_POKEMON + " LA", "Shiny Hunt - Custom Path",
        "ComputerControl/blob/master/Wiki/Programs/PokemonLA/ShinyHunt-CustomPath.md",
        "Repeatedly travel on a custom path to shiny hunt " + STRING_POKEMON + " around it.",
        ProgramControllerClass::StandardController_PerformanceClassSensitive,
        FeedbackType::VIDEO_AUDIO,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
class ShinyHuntCustomPath_Descriptor::Stats : public StatsTracker, public ShinyStatIncrementer{
public:
    Stats()
        : attempts(m_stats["Attempts"])
        , errors(m_stats["Errors"])
        , shinies(m_stats["Shinies"])
    {
        m_display_order.emplace_back("Attempts");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Shinies", HIDDEN_IF_ZERO);
    }
    virtual void add_shiny() override{
        shinies++;
    }

    std::atomic<uint64_t>& attempts;
    std::atomic<uint64_t>& errors;
    std::atomic<uint64_t>& shinies;
};
std::unique_ptr<StatsTracker> ShinyHuntCustomPath_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


ShinyHuntCustomPath::ShinyHuntCustomPath()
    : TIME_OF_DAY(
        "<b>Time of Day:</b><br>Reset time of day if <b>Reset Method</b> is Soft Reset. Use this to only hunt " + STRING_POKEMON
        + " at day or night, or to avoid visual inference errors on white snow at daytime."
    )
    , RUNS_PER_TIME_RESET(
        "<b>How Many Runs Before Resetting Time of Day:</b><br>To avoid too much time spent on resetting time of day, reset only every several runs.",
        LockMode::LOCK_WHILE_RUNNING,
        5, 1
    )
    , TEST_PATH(
        "<b>Test Path:</b><br>Run the path immediately on the map to test it.",
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
    , SHINY_DETECTED_ENROUTE(
        "Enroute Shiny Action",
        "This applies if a shiny is detected while you are ignoring shinies.",
        "0 ms"
    )
    , SHINY_DETECTED_DESTINATION(
        "Destination Shiny Action",
        "This applies if a shiny is detected while you are listening for shinies.",
        "0 ms"
    )
    , NOTIFICATION_STATUS("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS,
        &SHINY_DETECTED_DESTINATION.NOTIFICATIONS,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{

    PA_ADD_STATIC(SHINY_REQUIRES_AUDIO);
//    PA_ADD_OPTION(TRAVEL_LOCATION);
    PA_ADD_OPTION(PATH);
    PA_ADD_OPTION(RESET_METHOD);
    PA_ADD_OPTION(TIME_OF_DAY);
    PA_ADD_OPTION(RUNS_PER_TIME_RESET);
    PA_ADD_OPTION(TEST_PATH);
    PA_ADD_OPTION(SHINY_DETECTED_ENROUTE);
    PA_ADD_OPTION(SHINY_DETECTED_DESTINATION);
    PA_ADD_OPTION(NOTIFICATIONS);
}


void ShinyHuntCustomPath::do_non_listen_action(
    VideoStream& stream, ProControllerContext& context,
    const CustomPathTableRow& row
){
    stream.log("Execute action " + row.action.current_display());
    switch(row.action){
    case PathAction::CHANGE_MOUNT:
    {
        MountState mountState = MountState::NOTHING;
        switch(row.parameters.mount){
        case PathMount::WYRDEER:
            mountState = MountState::WYRDEER_ON;
            break;
        case PathMount::URSALUNA:
            mountState = MountState::URSALUNA_ON;
            break;
        case PathMount::BASCULEGION:
            mountState = MountState::BASCULEGION_ON;
            break;
        case PathMount::SNEASLER:
            mountState = MountState::SNEASLER_ON;
            break;
        case PathMount::BRAVIARY:
            mountState = MountState::BRAVIARY_ON;
            break;
        default:
            break;
        }

        if (mountState == MountState::NOTHING){
            dismount(stream, context);
        }else{
            change_mount(stream, context, mountState);
        }
        break;
    }
#if 0
    case PathAction::ROTATE_CAMERA:
    {
        if (row.camera_turn_ticks > 0){
            pbf_move_right_joystick(context, 255, 128, uint16_t(row.camera_turn_ticks), 0);
        }else if (row.camera_turn_ticks < 0){
            pbf_move_right_joystick(context, 0, 128, uint16_t(-row.camera_turn_ticks), 0);
        }
        break;
    }
#endif
    case PathAction::MOVE_FORWARD:
    {
        switch(row.parameters.move_speed){
        case PathSpeed::NORMAL_SPEED:
            pbf_move_left_joystick(context, 128, 0, row.parameters.move_forward, 0ms);
            break;
        case PathSpeed::SLOW_SPEED:
            pbf_move_left_joystick(context, 128, 64, row.parameters.move_forward, 0ms);
            break;
        case PathSpeed::RUN:
            pbf_controller_state(context, BUTTON_LCLICK, DPAD_NONE, 128, 0, 128, 128, row.parameters.move_forward);
            break;
        case PathSpeed::DASH:
            pbf_press_button(context, BUTTON_B, row.parameters.move_forward, 0ms);
            break;
        case PathSpeed::DASH_B_SPAM:
            pbf_mash_button(context, BUTTON_B, row.parameters.move_forward);
            break;
        case PathSpeed::DIVE:
            pbf_press_button(context, BUTTON_Y, row.parameters.move_forward, 0ms);
            break;
        }
        break;
    }
    case PathAction::MOVE_IN_DIRECTION:
    {
        uint8_t x = (uint8_t)((row.parameters.left_x + 1.0) * 127.5 + 0.5);
        uint8_t y = (uint8_t)((-row.parameters.left_y + 1.0) * 127.5 + 0.5);
        pbf_move_left_joystick(context, x, y, row.parameters.move_forward, 0ms);
        break;
    }
    case PathAction::CENTER_CAMERA:
    {
        pbf_mash_button(context, BUTTON_ZL, 200);
        break;
    }
    case PathAction::JUMP:
    {
        pbf_press_button(context, BUTTON_Y, 80ms, row.parameters.jump_wait);
        break;
    }
    case PathAction::WAIT:
    {
        pbf_wait(context, row.parameters.wait);
        break;
    }
    default:
        break;
    } // end switch action
    context.wait_for_all_requests();
}


void ShinyHuntCustomPath::run_path(SingleSwitchProgramEnvironment& env, ProControllerContext& context){

    std::vector<std::unique_ptr<CustomPathTableRow>> table = PATH.PATH.copy_snapshot();

    //  Check whether the user has set shiny sound listen action:
    {
        bool has_listen_action = false;
        for (const std::unique_ptr<CustomPathTableRow>& row : table){
            if (row->action == PathAction::START_LISTEN){
                has_listen_action = true;
                break;
            }
        }
        if (has_listen_action == false){
            throw UserSetupError(env.console, "No START LISTEN action specified.");
        }
    }

    ShinyHuntCustomPath_Descriptor::Stats& stats = env.current_stats<ShinyHuntCustomPath_Descriptor::Stats>();

    std::atomic<bool> listen_for_shiny(false);
    float shiny_coefficient = 1.0;
    OverworldShinyDetectedActionOption* shiny_action = nullptr;
    ShinySoundDetector shiny_detector(env.console, [&](float error_coefficient) -> bool{
        //  Warning: This callback will be run from a different thread than this function.
        stats.shinies++;
        shiny_coefficient = error_coefficient;
        if (listen_for_shiny.load(std::memory_order_acquire)){
            shiny_action = &SHINY_DETECTED_DESTINATION;
        }else{
            shiny_action = &SHINY_DETECTED_ENROUTE;
        }
        return on_shiny_callback(env, env.console, *shiny_action, error_coefficient);
    });

    int ret = run_until<ProControllerContext>(
        env.console, context,
        [&](ProControllerContext& context){
            for (const std::unique_ptr<CustomPathTableRow>& row : table){
                if (row->action == PathAction::START_LISTEN){
                    listen_for_shiny.store(true, std::memory_order_release);
                    continue;
                }else if (row->action == PathAction::END_LISTEN){
                    listen_for_shiny.store(false, std::memory_order_release);
                    continue;
                }

                do_non_listen_action(env.console, context, *row);
                context.wait_for_all_requests();
            }
        },
        {{shiny_detector}}
    );
    shiny_detector.throw_if_no_sound();
    if (ret == 0){
        on_shiny_sound(env, env.console, context, *shiny_action, shiny_coefficient);
    }
}


void ShinyHuntCustomPath::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    ShinyHuntCustomPath_Descriptor::Stats& stats = env.current_stats<ShinyHuntCustomPath_Descriptor::Stats>();

    //  Connect the controller.
    pbf_press_button(context, BUTTON_LCLICK, 5, 5);

    if (TEST_PATH){
        // Run the test path immediately
        env.log("Testing path...");
        run_path(env, context);
        return;
    }

    // How many runs so far after last time reset
    uint32_t time_reset_run_count = 0;

    bool fresh_from_reset = false;
    while (true){
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS);
        try{
            stats.attempts++;

            const TravelLocation location = PATH.travel_location();
            goto_camp_from_jubilife(env, env.console, context, location, fresh_from_reset);
            run_path(env, context);
            ++time_reset_run_count;

            if(RESET_METHOD == ResetMethod::SoftReset){
                env.console.log("Resetting by closing the game.");
                pbf_press_button(context, BUTTON_HOME, 160ms, GameSettings::instance().GAME_TO_HOME_DELAY0);
                fresh_from_reset = reset_game_from_home(
                    env, env.console, context,
                    ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST
                );
            }else{
                env.console.log("Resetting by going to village.");
                goto_camp_from_overworld(env, env.console, context);

                // Now we are at this camp
                const Camp camp = map_region_default_camp(location.region);
                TimeOfDay target_time = TIME_OF_DAY;
                if (target_time != TimeOfDay::NONE && time_reset_run_count >= RUNS_PER_TIME_RESET){
                    env.log("Reset time to " + TIME_OF_DAY_NAMES[int(target_time)]);
                    time_reset_run_count = 0;
                    change_time_of_day_at_tent(env.console, context, target_time, camp);
                    // Reset location again since we now are at the tent, not the camp warp spot
                    goto_camp_from_overworld(env, env.console, context);
                }
                goto_professor(env.console.logger(), context, camp);
                from_professor_return_to_jubilife(env, env.console, context);
            }

        }catch (OperationFailedException& e){
            stats.errors++;
            e.send_notification(env, NOTIFICATION_ERROR_RECOVERABLE);

            time_reset_run_count = 0;
            pbf_press_button(context, BUTTON_HOME, 160ms, GameSettings::instance().GAME_TO_HOME_DELAY0);
            fresh_from_reset = reset_game_from_home(
                env, env.console, context,
                ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST
            );
        }

    }

    env.update_stats();
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}





}
}
}
