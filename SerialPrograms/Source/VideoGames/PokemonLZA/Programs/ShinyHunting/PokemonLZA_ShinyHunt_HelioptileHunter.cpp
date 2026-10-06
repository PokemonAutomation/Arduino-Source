/*  Shiny Hunt - Helioptile Hunter
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <array>
#include <cstddef>
#include <string>
#include "Common/Cpp/Options/ConfigOption.h"
#include "CommonFramework/Exceptions/OperationFailedExceptionWithScreenshot.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ControllerButtons.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "Pokemon/Pokemon_Strings.h"
#include "VideoGames/PokemonLZA/Inference/PokemonLZA_ButtonDetector.h"
#include "VideoGames/PokemonLZA/Inference/PokemonLZA_DayNightStateDetector.h"
#include "VideoGames/PokemonLZA/Inference/PokemonLZA_WeatherDetector.h"
#include "VideoGames/PokemonLZA/Programs/PokemonLZA_BasicNavigation.h"
#include "VideoGames/PokemonLZA/Programs/PokemonLZA_FastTravelNavigation.h"
#include "PokemonLZA_ShinyHunt_HelioptileHunter.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

using namespace Pokemon;





ShinyHunt_HelioptileHunter_Descriptor::ShinyHunt_HelioptileHunter_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLZA:ShinyHunt-HelioptileHunter",
        STRING_POKEMON + " LZA", "Helioptile Hunter",
        "Programs/PokemonLZA/ShinyHunt-HelioptileHunter.html",
        "Hunts for Helioptile in Wild Zone by entering and resetting, checking for the right weather.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

class ShinyHunt_HelioptileHunter_Descriptor::Stats : public StatsTracker{
public:
    Stats()
        : cycles(m_stats["Cycles"])
        , loops(m_stats["Loops"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Cycles");
        m_display_order.emplace_back("Loops");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }

    std::atomic<uint64_t>& cycles;
    std::atomic<uint64_t>& loops;
    std::atomic<uint64_t>& errors;
};

std::unique_ptr<StatsTracker> ShinyHunt_HelioptileHunter_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

ShinyHunt_HelioptileHunter::ShinyHunt_HelioptileHunter()
    : END_AFTER_CYCLE(
        "<b>How many day/night cycles before stopping. 0 for never stop.</b><br>"
        "<br>"
        "Each day/night cycle is roughly 65 tries.",
        LockMode::LOCK_WHILE_RUNNING,
        0, 0
    )
    , NOTIFICATION_STATUS("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(END_AFTER_CYCLE);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void bench_loop(SingleSwitchProgramEnvironment& env, ProControllerContext& context, size_t quantity){
    for (size_t i = 0; i < quantity; i++){
        sit_on_bench(env.console, context);
        pbf_move_left_joystick(context, {0, -1}, 500ms, 200ms);
    }
}

struct WeatherTimeState{
    bool correct_weather;
    bool daytime;
};

WeatherTimeState get_weather_time_state(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    bool close_map
){
    open_map(env.console, context, false, true);
    context.wait_for_all_requests();

    // zoom fully in
    pbf_move_right_joystick(context, {0, 1}, 900ms, 120ms);
    context.wait_for_all_requests();
    // hide icons
    pbf_press_button(context, BUTTON_MINUS, 80ms, 120ms);
    context.wait_for_all_requests();

    VideoSnapshot screen = env.console.video().snapshot();
    // Log every weather type so uploaded user logs distinguish unsuitable
    // weather from missed or ambiguous detections. These small-region checks
    // run between controller sequences; retain them for support troubleshooting.
    // Keep the Sunny-or-Clear acceptance rule, including multiple matches.
    const std::array<WeatherIconType, 6> types = {
        WeatherIconType::Sunny, WeatherIconType::Clear, WeatherIconType::Rain,
        WeatherIconType::Cloudy, WeatherIconType::Foggy, WeatherIconType::Rainbow,
    };
    const std::array<const char*, 6> names = {
        "Sunny", "Clear", "Rain", "Cloudy", "Foggy", "Rainbow",
    };
    std::array<bool, 6> matches{};
    std::string detected;
    std::string detection_map;
    size_t match_count = 0;
    for (size_t i = 0; i < types.size(); i++){
        WeatherIconDetector detector(types[i], &env.console.overlay());
        matches[i] = detector.detect(screen);
        if (i > 0){
            detection_map += ",";
        }
        detection_map += std::string(names[i]) + "=" + (matches[i] ? "1" : "0");
        if (matches[i]){
            if (match_count++ > 0){
                detected += ",";
            }
            detected += names[i];
        }
    }
    if (match_count == 0){
        detected = "Unknown";
    }

    DayNightStateDetector dayNightDetector(&env.console.overlay());
    dayNightDetector.detect(screen);
    bool daytime = dayNightDetector.state() != DayNightState::NIGHT;
    bool correct_weather = matches[0] || matches[1];

    // Mirror the detector's terrain sample so the day/night threshold can be
    // checked against capture brightness without changing the detector itself.
    const ImageStats terrain = image_stats(
        extract_box_reference(screen, ImageFloatBox(0.30, 0.55, 0.15, 0.18))
    );
    const double blue_ratio = terrain.average.b /
        (terrain.average.r + terrain.average.g + terrain.average.b);
    auto& stats = env.current_stats<ShinyHunt_HelioptileHunter_Descriptor::Stats>();
    env.log("[Helioptile][Weather] cycle=" + std::to_string(stats.cycles.load() + 1)
        + " completed_loops=" + std::to_string(stats.loops.load())
        + " phase=" + (close_map ? "bench" : "hunt")
        + " detected=" + detected
        + " ambiguous=" + (match_count > 1 ? "1" : "0")
        + " matches={" + detection_map + "}"
        + " time=" + (daytime ? "Day" : "Night")
        + " blue_ratio=" + std::to_string(blue_ratio)
        + " night_threshold=0.36"
        + " eligible=" + (correct_weather && daytime ? "1" : "0")
        + " frame=" + std::to_string(screen->width()) + "x" + std::to_string(screen->height())
    );

    if (close_map){
        context.wait_for_all_requests();
        pbf_press_button(context, BUTTON_PLUS, 500ms, 500ms);
        context.wait_for_all_requests();
    }

    return {correct_weather, daytime};
}

void find_weather(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    WeatherTimeState state = get_weather_time_state(env, context, true);
    context.wait_for_all_requests();
    env.log("[Helioptile][Decision] action=initial_bench_reroll sits="
        + std::to_string(state.daytime ? 2 : 1));
    bench_loop(env, context, (state.daytime) ? 2 : 1);

    size_t weather_attempt = 0;
    while (true){
        env.log("[Helioptile][Bench] attempt=" + std::to_string(++weather_attempt));
        state = get_weather_time_state(env, context, true);
        if (state.correct_weather && state.daytime){
            break;
        }
        env.log("[Helioptile][Decision] action=bench_reroll sits=2 reason="
            + std::string(!state.correct_weather ? "weather" : "night"));
        bench_loop(env, context, 2);
    }
    env.log("[Helioptile][Decision] action=weather_found destination=WildZone14");
    context.wait_for_all_requests();
}

void reach_bench(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    //Go to poke center
    FastTravelState result = open_map_and_fly_to(
        env.console, context, Language::English,
        Location::MAGENTA_POKEMON_CENTER, false, true
    );

    if (result != FastTravelState::SUCCESS) {
        env.log(
            "Fast travel to Magenta failed. State = "
            + std::to_string((int)result)
        );

        OperationFailedExceptionWithScreenshot::fire(
            ErrorReportMode::SEND_ERROR_REPORT,
            "Failed to fast travel to Magenta Pokemon Center.",
            env.console
        );
    }
    context.wait_for_all_requests();
    //Go to bench
    pbf_move_left_joystick(context, {-1, 0},  700ms, 200ms);
    pbf_move_left_joystick(context, {0, +1}, 500ms, 200ms);
}

void reach_gate(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context
){
    ButtonWatcher buttonA(COLOR_RED, ButtonType::ButtonA, {0.3, 0.2, 0.4, 0.7}, &env.console.overlay());
    run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context){
            for (int c = 0; c < 30; c++){
                ssf_press_button(context, BUTTON_B, 0ms, 2s, 0ms);
                pbf_move_left_joystick(context, {0, +1}, 2s, 200ms);
            }
        },
        {{buttonA}}
    );
    env.log("Detected button A. At Wild Zone gate.");
    env.console.overlay().add_log("Detect Entrance");
}

void warp_wild_zone_14(SingleSwitchProgramEnvironment& env, ProControllerContext& context
) {
    FastTravelState result = open_map_and_fly_to(
        env.console, context, Language::English,
        Location::WILD_ZONE_14, false, true
    );

    if (result != FastTravelState::SUCCESS) {
        OperationFailedExceptionWithScreenshot::fire(
            ErrorReportMode::SEND_ERROR_REPORT,
            "Failed to fast travel to Wild Zone 14.",
            env.console
        );
    }
}

void execute_fixed_routine(SingleSwitchProgramEnvironment& env, ConsoleHandle& console, ProControllerContext& context, EventNotificationOption& settings){
    context.wait_for_all_requests();
    console.overlay().add_log("Starting routine");

    reach_gate(env, context);
    context.wait_for_all_requests();
    pbf_press_button(context, BUTTON_A, 500ms, 1500ms);

    //moving forward
    ssf_press_button(context, BUTTON_B, 0ms, 2000ms, 0ms);
    pbf_move_left_joystick(context, {0, +1}, 2000ms, 700ms);
    context.wait_for_all_requests();
    send_program_status_notification(env, settings, "", env.console.video().snapshot());
    ssf_press_button(context, BUTTON_B, 0ms, 2500ms, 0ms);
    pbf_move_left_joystick(context, {0, +1}, 2500ms, 500ms);

    //moving back
    ssf_press_button(context, BUTTON_B, 0ms, 5s, 0ms);
    pbf_move_left_joystick(context, {0, -1}, 5s, 500ms);

    //leave WZ
    pbf_press_button(context, BUTTON_A, 500ms, 1500ms);
    context.wait_for_all_requests();
}

void ShinyHunt_HelioptileHunter::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    //  Connect the controller.
    require_player(env.console, context, BUTTON_L);

    ShinyHunt_HelioptileHunter_Descriptor::Stats& stats = env.current_stats<ShinyHunt_HelioptileHunter_Descriptor::Stats>();

    while(true){

        if (END_AFTER_CYCLE.current_value() > 0 &&
            END_AFTER_CYCLE.current_value() == stats.cycles.load(std::memory_order_relaxed)
        ){
            go_home(env.console, context);
            send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
            break;
        }

        int hunt_loops = 0;
        std::string cycle_end_reason = "loop_limit";
        while (hunt_loops < 65){
            // Begin each cycle by re-rolling weather at the bench. A cycle ends
            // after 65 completed hunt loops or when the weather/time check fails.
            if (hunt_loops == 0){
                env.log("[Helioptile][Decision] action=reach_bench reason=cycle_start");
                reach_bench(env, context);
                find_weather(env, context);
                warp_wild_zone_14(env, context);
                hunt_loops = 0;
            }else{
                WeatherTimeState state = get_weather_time_state(env, context, false);
                if (state.correct_weather && state.daytime){
                    env.log("[Helioptile][Decision] action=continue_hunt");
                    // Zoom fully out before moving map cursor for fast travel.
                    pbf_move_right_joystick(context, {0, -1}, 900ms, 120ms);
                    // Re-show icons before moving the map cursor to the destination.
                    pbf_press_button(context, BUTTON_MINUS, 80ms, 120ms);
                    //these extra waits help prevent early execution of the map move which sometimes fired early during testing
                    context.wait_for_all_requests();
                    pbf_wait(context, 100ms);
                    move_map_cursor_from_entrance_to_zone(env.console, context, Location::WILD_ZONE_14);
                    FastTravelState result = fly_from_map(env.console, context);

                    switch (result) {
                    case FastTravelState::SUCCESS:
                        wait_until_overworld(env.console, context);
                        break;

                    default:
                        OperationFailedExceptionWithScreenshot::fire(
                            ErrorReportMode::SEND_ERROR_REPORT,
                            "Failed to fast travel back to Wild Zone 14 after weather check.",
                            env.console
                        );
                    }
                }else{
                    cycle_end_reason = !state.correct_weather ? "weather" : "night";
                    env.log("[Helioptile][Decision] action=end_cycle reason=" + cycle_end_reason);
                    pbf_press_button(context, BUTTON_PLUS, 500ms, 500ms);
                    context.wait_for_all_requests();
                    break;
                }
            }

            env.log(
                "[Helioptile][Loop] cycle=" + std::to_string(stats.cycles.load(std::memory_order_relaxed) + 1)
                + " cycle_loop=" + std::to_string(hunt_loops + 1)
                + " total_loop=" + std::to_string(stats.loops.load() + 1)
            );
            execute_fixed_routine(env,env.console, context, NOTIFICATION_STATUS);

            stats.loops++;
            hunt_loops++;
            env.update_stats();
        }
        env.log(
            "[Helioptile][Decision] action=cycle_complete cycle="
            + std::to_string(stats.cycles.load(std::memory_order_relaxed) + 1)
            + " completed_loops=" + std::to_string(hunt_loops)
            + " reason=" + cycle_end_reason
        );
        stats.cycles++;
        env.update_stats();
    }
}
}
}
}
