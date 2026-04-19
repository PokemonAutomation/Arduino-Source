/*  Shiny Hunt - Bench Sit
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLA/Inference/Sounds/PokemonLA_ShinySoundDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_DayNightStateDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_ButtonDetector.h"
#include "PokemonLZA/Programs/PokemonLZA_BasicNavigation.h"
#include "PokemonLZA_ShinyHunt_BenchSit.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

using namespace Pokemon;





ShinyHunt_BenchSit_Descriptor::ShinyHunt_BenchSit_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLZA:ShinyHunt-BenchSit",
        STRING_POKEMON + " LZA", "Bench Sit",
        "Programs/PokemonLZA/ShinyHunt-BenchSit.html",
        "Shiny hunt by repeatedly sitting on a bench to reset spawns.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
class ShinyHunt_BenchSit_Descriptor::Stats : public StatsTracker{
public:
    Stats()
        : resets(m_stats["Bench Sits"])
        , shinies(m_stats["Shiny Sounds"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Bench Sits");
        m_display_order.emplace_back("Shiny Sounds");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);

        m_aliases["Shinies"] = "Shiny Sounds";
        m_aliases["Shinies Detected"] = "Shiny Sounds";
    }

    std::atomic<uint64_t>& resets;
    std::atomic<uint64_t>& shinies;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> ShinyHunt_BenchSit_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}





ShinyHunt_BenchSit::ShinyHunt_BenchSit()
    : WALK_DIRECTION(
        "<b>Run Direction:</b><br>The direction of running after each day change to increase the spawn radius.",
        {
            {0, "forward", "Forward"},
            {1, "left", "Turn Left"},
            {2, "right", "Turn Right"},
        },
        LockMode::UNLOCK_WHILE_RUNNING,
        0
    )
    , WALK_FORWARD_DURATION(
        "<b>Run Forward Duration</b><br>"
        "Run forward and backward for this long after each day change to "
        "increase the spawn radius. Set to zero to disable this.",
        LockMode::UNLOCK_WHILE_RUNNING,
        "2000 ms"
    )
    , PERIODIC_SAVE(
        "<b>Periodically Save:</b><br>"
        "Save the game every this many bench sits. This reduces the loss to game crashes. Set to zero to disable. Saving will be unsuccessful if you are under attack",
        LockMode::UNLOCK_WHILE_RUNNING,
        100,
        0
    )
    , PERIODIC_TIME_CHECK(
        "<b>Periodically Check Time:</b><br>"
        "Re-check day/night state every this many bench sits. "
        "Set to 0 to check every cycle.",
        LockMode::UNLOCK_WHILE_RUNNING,
        10,
        0
    )
    , DAY_NIGHT_FILTER(
        "Run Forward Only During Day/Night:",
        LockMode::LOCK_WHILE_RUNNING,
        GroupOption::EnableMode::DEFAULT_DISABLED
    )
    , DAY_FILTER_MODE(
        "Time filter",
        {
        {0, "day", "Day"},
        {1, "night", "Night"},
        },
        LockMode::UNLOCK_WHILE_RUNNING,
        0
    )
    , SHINY_DETECTED(
        "Shiny Detected", "",
        "2000 ms",
        ShinySoundDetectedAction::NOTIFY_ON_FIRST_ONLY
    )
    , NOTIFICATION_STATUS("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS,
        &SHINY_DETECTED.NOTIFICATIONS,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_STATIC(SHINY_REQUIRES_AUDIO);
    if (PreloadSettings::instance().DEVELOPER_MODE){
        PA_ADD_OPTION(WALK_DIRECTION);
    }
    PA_ADD_OPTION(WALK_FORWARD_DURATION);
    PA_ADD_OPTION(PERIODIC_SAVE);
    PA_ADD_OPTION(PERIODIC_TIME_CHECK);
    PA_ADD_OPTION(DAY_NIGHT_FILTER);
    DAY_NIGHT_FILTER.add_option(DAY_FILTER_MODE, "FilterMode");
    PA_ADD_OPTION(SHINY_DETECTED);
    PA_ADD_OPTION(NOTIFICATIONS);
}


void run_back_until_found_bench(
    SingleSwitchProgramEnvironment& env, ProControllerContext& context
){
    ButtonWatcher buttonA(
        COLOR_RED,
        ButtonType::ButtonA,
        {0.486, 0.477, 0.115, 0.5},
        &env.console.overlay()
    );

    int ret = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context){
            ssf_press_button(context, BUTTON_B, 0ms, 800ms, 0ms);
            pbf_move_left_joystick(context, {0, -1}, 800ms, 200ms);
            pbf_press_button(context, BUTTON_L, 160ms, 160ms);

            //  Can't just hold it down since sometimes it doesn't register.
            for (int c = 0; c < 10; c++){
                pbf_move_right_joystick(context, {-1, 0}, 800ms, 200ms);
                pbf_press_button(context, BUTTON_L, 160ms, 0ms);
                pbf_move_left_joystick(context, {0, +1}, 840ms, 800ms);
            }
        },
        {buttonA}
    );

    switch (ret){
    case 0:
        env.console.log("Detected floating A button...");
        break;
    default:
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "run_back_until_found_bench(): Unable to detect bench after multiple attempts.",
            env.console
        );
    }
}
bool ShinyHunt_BenchSit::should_run_based_on_day_night(
    const ImageViewRGB32& frame, VideoOverlay& overlay){

    if ((Milliseconds)WALK_FORWARD_DURATION == Milliseconds::zero()){
        return true;
    }
    if (!DAY_NIGHT_FILTER.enabled()){
        return true;
    }
    if (!m_day_night_detector){
        m_day_night_detector = std::make_unique<DayNightStateDetector>(&overlay);
    }
    if (!m_day_night_detector->detect(frame)){
        return true;
    }
    DayNightState current_state = m_day_night_detector->state();
    if (current_state == DayNightState::NIGHT) {
        overlay.add_log("Day/Night Detector: NIGHT", COLOR_CYAN);
    } else {
        overlay.add_log("Day/Night Detector: DAY", COLOR_YELLOW);
    }
    size_t filter_mode = DAY_FILTER_MODE.current_value();
    if (filter_mode == 0){
        return current_state == DayNightState::DAY;
    }
    else if (filter_mode == 1){
        return current_state == DayNightState::NIGHT;
    }
    return true;
}
void ShinyHunt_BenchSit::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    ShinyHunt_BenchSit_Descriptor::Stats& stats = env.current_stats<ShinyHunt_BenchSit_Descriptor::Stats>();

    ShinySoundHandler shiny_sound_handler(SHINY_DETECTED);

    DayNightState cached_time = DayNightState::DAY;
    bool cached_time_initialized = false;
    uint32_t rounds_since_time_check = 0;

    PokemonLA::ShinySoundDetector shiny_detector(env.console, [&](float error_coefficient) -> bool{
        //  Warning: This callback will be run from a different thread than this function.
        stats.shinies++;
        env.update_stats();
        env.console.overlay().add_log("Shiny Sound Detected!", COLOR_YELLOW);
        return shiny_sound_handler.on_shiny_sound(
            env, env.console,
            stats.shinies,
            error_coefficient
        );
    });

    run_until<ProControllerContext>(
        env.console, context,
        [&](ProControllerContext& context){
            for (uint32_t rounds_since_last_save = 0;; rounds_since_last_save++){
                send_program_status_notification(env, NOTIFICATION_STATUS);
                sit_on_bench(env.console, context);
                shiny_sound_handler.process_pending(context);
                stats.resets++;
                env.update_stats();
                if (cached_time_initialized){
                    cached_time =
                        cached_time == DayNightState::DAY
                            ? DayNightState::NIGHT
                            : DayNightState::DAY;
                    rounds_since_time_check++;
                }
                uint32_t periodic_save = PERIODIC_SAVE;
                if (periodic_save != 0 && rounds_since_last_save >= periodic_save){
                    bool save_successful = save_game_to_menu(env.console, context);
                    pbf_mash_button(context, BUTTON_B, 2000ms);
                    if (save_successful){
                        env.console.overlay().add_log("Game Saved Successfully", COLOR_BLUE);
                        rounds_since_last_save = 0;
                    }else{
                        env.console.overlay().add_log("Game Save Failed. Will attempt to save after the next reset.", COLOR_RED);
                    }
                }

                Milliseconds duration = WALK_FORWARD_DURATION;

                // Only open map if movement is enabled AND a filter is active
                bool need_time_check = true;

                uint32_t periodic_time_check = PERIODIC_TIME_CHECK;

                if (periodic_time_check > 0 && cached_time_initialized){
                    need_time_check =
                        rounds_since_time_check >= periodic_time_check;
                }

                if (
                    duration > Milliseconds::zero() &&
                    DAY_NIGHT_FILTER.enabled() &&
                    (!cached_time_initialized || need_time_check)
                ){
                    open_map(env.console, context, false, true);
                    context.wait_for_all_requests();
                    pbf_wait(context, 180ms);
                    // zoom fully in
                    pbf_move_right_joystick(context, {0, 1}, 900ms, 120ms);
                    context.wait_for_all_requests();
                    pbf_wait(context, 120ms);
                    // hide icons
                    pbf_press_button(context, BUTTON_MINUS, 80ms, 120ms);
                    context.wait_for_all_requests();
                    pbf_wait(context, 160ms);
                    // double snapshot
                    ImageViewRGB32 frame1 =
                        env.console.video().snapshot();
                    pbf_wait(context, 120ms);
                    ImageViewRGB32 frame2 =
                        env.console.video().snapshot();
                    should_run_based_on_day_night(frame1, env.console.overlay());
                    should_run_based_on_day_night(frame2, env.console.overlay());
                    if (m_day_night_detector){
                        cached_time = m_day_night_detector->state();
                        cached_time_initialized = true;
                        rounds_since_time_check = 0;
                    }

                    // close map
                    pbf_wait(context, 120ms);
                    pbf_press_button(context, BUTTON_PLUS, 500ms, 120ms);
                    context.wait_for_all_requests();
                    pbf_wait(context, 150ms);
                }
                // filtering
                if (DAY_NIGHT_FILTER.enabled()){

                    size_t filter_mode = DAY_FILTER_MODE.current_value();

                    bool predicted_ok =
                        filter_mode == 0
                            ? cached_time == DayNightState::DAY
                            : cached_time == DayNightState::NIGHT;

                    if (!predicted_ok){

                        env.console.overlay().add_log(
                            "Skipping move (predicted wrong time of day)",
                            COLOR_ORANGE
                            );

                        run_back_until_found_bench(env, context);

                        shiny_sound_handler.process_pending(context);

                        continue;
                    }
                }

                // movement
                if (duration > Milliseconds::zero()){
                    if (WALK_DIRECTION.current_value() == 0){ //forward
                        env.console.overlay().add_log("Move Forward");
                        ssf_press_button(context, BUTTON_B, 0ms, 2*duration, 0ms);
                        pbf_move_left_joystick(context, {0, +1}, duration, 0ms);
                        pbf_move_left_joystick(context, {0, -1}, duration, 0ms);
                        run_back_until_found_bench(env, context);
                    }
                    else if (WALK_DIRECTION.current_value() == 1){//left
                        env.console.overlay().add_log("Move Left");
                        ssf_press_button(context, BUTTON_B, 0ms, duration, 0ms);
                        pbf_move_left_joystick(context, {-1, 0}, duration, 0ms);
                        pbf_press_button(context, BUTTON_L, 130ms, 450ms);
                        ssf_press_button(context, BUTTON_B, 0ms, duration, 0ms);
                        pbf_move_left_joystick(context, {0, -1}, duration, 0ms);
                        pbf_move_left_joystick(context, {-1, 0}, 150ms, 0ms);
                    }
                    else if (WALK_DIRECTION.current_value() == 2){ //right
                        env.console.overlay().add_log("Move Right");
                        ssf_press_button(context, BUTTON_B, 0ms, duration, 0ms);
                        pbf_move_left_joystick(context, {+1, 0}, duration, 0ms);
                        pbf_press_button(context, BUTTON_L, 130ms, 450ms);
                        ssf_press_button(context, BUTTON_B, 0ms, duration, 0ms);
                        pbf_move_left_joystick(context, {0, -1}, duration +150ms, 0ms);
                        pbf_move_left_joystick(context, {+1, 0}, 150ms, 0ms);
                    }
                }else{
                    run_back_until_found_bench(env, context);
                }
                shiny_sound_handler.process_pending(context);
            }
        },
        {shiny_detector}
    );

    //  Shiny sound detected and user requested stopping the program when
    //  detected shiny sound.
    shiny_sound_handler.process_pending(context);

    go_home(env.console, context);
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}














}
}
}
