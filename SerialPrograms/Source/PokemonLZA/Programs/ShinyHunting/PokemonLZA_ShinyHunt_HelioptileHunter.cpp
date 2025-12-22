/*  Shiny Hunt - Helioptile Hunter
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Options/ConfigOption.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ControllerButtons.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLZA/Inference/PokemonLZA_ButtonDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_WeatherDetector.h"
#include "PokemonLZA_ShinyHunt_HelioptileHunter.h"
#include "PokemonLZA/Programs/PokemonLZA_BasicNavigation.h"
#include "PokemonLZA/Programs/PokemonLZA_Locations.h"
#include <cstddef>
#include <string>

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
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        {}
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
    : END_AFTER_CYCLE("<b>How Many cycles before stopping. 0 for never stop.</b>",
        LockMode::LOCK_WHILE_RUNNING,
        0, 0, 32*30
    )
    , NOTIFICATION_STATUS("Status Update", true, false,   ImageAttachmentMode::JPG, {"Notifs"}, std::chrono::seconds(3600))
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

bool proper_weather(SingleSwitchProgramEnvironment& env, ProControllerContext& context) {
    
    open_map(env.console, context, true);

    WeatherIconDetector sunnyDetector(
        WeatherIconType::Sunny,
        &env.console.overlay()
    );

    WeatherIconDetector clearDetector(
        WeatherIconType::Clear,
        &env.console.overlay()
    );
    VideoSnapshot screen = env.console.video().snapshot();
    
    return (sunnyDetector.detect(screen) || clearDetector.detect(screen));
}

void bench_loop(SingleSwitchProgramEnvironment& env, ProControllerContext& context, size_t quantity) {
    for(size_t i = 0; i < quantity; i++) {
        sit_on_bench(env.console, context);
        pbf_move_left_joystick(context, {0, -1}, 500ms, 200ms);
    }
}

void find_weather(SingleSwitchProgramEnvironment& env, ProControllerContext& context, bool is_night_time) {
    context.wait_for_all_requests();
    env.log("Starting weather loop");
    bench_loop(env, context, (is_night_time) ? 1 : 2);

    while (!proper_weather(env, context)) {   
        env.log("Weather not found"); 
        pbf_press_button(context, BUTTON_PLUS, 500ms, 500ms);
        bench_loop(env, context, 2);
    }
    env.log("Weather found");
    context.wait_for_all_requests();
}

void reach_bench(SingleSwitchProgramEnvironment& env, ProControllerContext& context) {
    //Go to poke center
    pbf_move_left_joystick(context, 110, 0, 100ms, 200ms);
    pbf_press_button(context, BUTTON_A, 500ms, 500ms);
    pbf_press_button(context, BUTTON_A, 500ms, 500ms);
    pbf_wait(context, 3000ms);
    //Go to bench
    pbf_move_left_joystick(context, {-1, 0},  700ms, 200ms);
    pbf_move_left_joystick(context, 128, 0, 500ms, 200ms);
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
                pbf_move_left_joystick(context, 128, 0, 2s, 200ms);
            }
        },
        {{buttonA}}
    );
    env.log("Detected button A. At Wild Zone gate.");
    env.console.overlay().add_log("Detect Entrance");
}

void reach_wild_zone(SingleSwitchProgramEnvironment& env, ProControllerContext& context) {
    pbf_move_left_joystick(context, 140, 255, 200ms, 200ms);
    pbf_press_button(context, BUTTON_A, 500ms, 500ms);
    pbf_press_button(context, BUTTON_A, 500ms, 500ms);
    pbf_wait(context, 2000ms);
}

void execute_fixed_routine(SingleSwitchProgramEnvironment& env, ConsoleHandle& console, ProControllerContext& context, EventNotificationOption& settings){
    context.wait_for_all_requests();
    console.overlay().add_log("Starting routine");

    reach_gate(env, context);
    context.wait_for_all_requests();
    pbf_press_button(context, BUTTON_A, 500ms, 1500ms);

    //moving forward
    ssf_press_button(context, BUTTON_B, 0ms, 2000ms, 0ms);
    pbf_move_left_joystick(context, 128, 0, 2000ms, 700ms);
    context.wait_for_all_requests();
    send_program_status_notification(env, settings, "", env.console.video().snapshot());
    ssf_press_button(context, BUTTON_B, 0ms, 2500ms, 0ms);
    pbf_move_left_joystick(context, 128, 0, 2500ms, 500ms);

    //moving back
    ssf_press_button(context, BUTTON_B, 0ms, 5s, 0ms);
    pbf_move_left_joystick(context, {0, -1}, 5s, 500ms);

    //leave WZ
    pbf_press_button(context, BUTTON_A, 500ms, 1500ms);
    context.wait_for_all_requests();
}

void ShinyHunt_HelioptileHunter::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);
    ShinyHunt_HelioptileHunter_Descriptor::Stats& stats = env.current_stats<ShinyHunt_HelioptileHunter_Descriptor::Stats>();
    
    // This routine do not care for day/night change as is supposed to stop before time change.
    while(true){
        
        if (END_AFTER_CYCLE.current_value() > 0 && END_AFTER_CYCLE.current_value() == stats.cycles.load()) {
            go_home(env.console, context);
            send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
            break;
        }

        int hunt_loops = 0;
        while (hunt_loops < 65) {
            if (!proper_weather(env,context) || (hunt_loops == 0)) {
                env.log("Not correct weather");
                reach_bench(env, context);
                find_weather(env, context, false);
                reach_wild_zone(env, context);
                hunt_loops = 0;
            } else {
                env.log("Correct weather. Continuing");
                move_map_cursor_from_entrance_to_zone(env.console, context, WildZone::WILD_ZONE_14);
                fly_from_map(env.console, context);
            }   

            execute_fixed_routine(env,env.console, context, NOTIFICATION_STATUS);

            stats.loops++;
            hunt_loops++;
            env.update_stats();
        }
        stats.cycles++;
        env.update_stats();
    }
}
}
}
}
