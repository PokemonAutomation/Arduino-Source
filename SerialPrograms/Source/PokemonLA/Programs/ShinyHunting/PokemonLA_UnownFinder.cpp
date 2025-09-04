/*  Unown Finder
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
#include "PokemonLA/PokemonLA_TravelLocations.h"
#include "PokemonLA/Inference/Sounds/PokemonLA_ShinySoundDetector.h"
#include "PokemonLA/Programs/PokemonLA_MountChange.h"
#include "PokemonLA/Programs/PokemonLA_GameEntry.h"
#include "PokemonLA/Programs/PokemonLA_RegionNavigation.h"
#include "PokemonLA/Programs/ShinyHunting/PokemonLA_UnownFinder.h"


namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{
    using namespace Pokemon;



UnownFinder_Descriptor::UnownFinder_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLA:UnownFinder",
        STRING_POKEMON + " LA", "Unown Hunter",
        "ComputerControl/blob/master/Wiki/Programs/PokemonLA/UnownHunter.md",
        "Constantly reset to find a Shiny Unown or any Shiny in the path.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::VIDEO_AUDIO,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
class UnownFinder_Descriptor::Stats : public StatsTracker, public ShinyStatIncrementer{
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
std::unique_ptr<StatsTracker> UnownFinder_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}




UnownFinder::UnownFinder()
    : SHINY_DETECTED_ENROUTE(
        "Enroute Shiny Action",
        "This applies if a shiny is detected while enroute to the ruins.",
        "0 ms"
    )
    , SHINY_DETECTED_DESTINATION(
        "Destination Shiny Action",
        "This applies if a shiny is detected inside the ruins.",
        "0 ms"
    )
    , NOTIFICATION_STATUS("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS,
        &SHINY_DETECTED_ENROUTE.NOTIFICATIONS,
        &SHINY_DETECTED_DESTINATION.NOTIFICATIONS,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_STATIC(SHINY_REQUIRES_AUDIO);
    PA_ADD_OPTION(SHINY_DETECTED_ENROUTE);
    PA_ADD_OPTION(SHINY_DETECTED_DESTINATION);
    PA_ADD_OPTION(NOTIFICATIONS);
}


void ruins_entrance_route(ProControllerContext& context){
    pbf_wait(context, (uint16_t)(0.5 * TICKS_PER_SECOND));
    pbf_move_left_joystick(context, 139, 120, 10, 10);
    pbf_wait(context, (uint16_t)(1.3 * TICKS_PER_SECOND));

    pbf_press_button(context, BUTTON_B, (uint16_t)(9.5 * TICKS_PER_SECOND), 10);
    pbf_wait(context, (uint16_t)(0.8 * TICKS_PER_SECOND));
    pbf_move_left_joystick(context, 110, 90, 20, 10);

    pbf_press_dpad(context, DPAD_LEFT, 10, 10);
    pbf_press_button(context, BUTTON_PLUS, 10, 10);
}

void enter_ruins(ProControllerContext& context){
    pbf_press_button(context, BUTTON_B, (uint16_t)(4 * TICKS_PER_SECOND), 10);
    pbf_wait(context, (uint16_t)(1.5 * TICKS_PER_SECOND));
    pbf_move_left_joystick(context, 128, 255, 10, 0);
    pbf_press_button(context, BUTTON_B, (uint16_t)(2 * TICKS_PER_SECOND), 10);
}


void UnownFinder::run_iteration(
    SingleSwitchProgramEnvironment& env, ProControllerContext& context,
    bool fresh_from_reset
){
    UnownFinder_Descriptor::Stats& stats = env.current_stats<UnownFinder_Descriptor::Stats>();

    stats.attempts++;

    goto_camp_from_jubilife(
        env, env.console, context,
        TravelLocations::instance().Mirelands_Mirelands,
        fresh_from_reset
    );

    change_mount(env.console, context, MountState::BRAVIARY_ON);

    // Start path
    env.console.log("Beginning Shiny Detection...");
    {
        float shiny_coefficient = 1.0;
        std::atomic<OverworldShinyDetectedActionOption*> shiny_action(&SHINY_DETECTED_ENROUTE);
        WallClock destination_time = WallClock::max();

        ShinySoundDetector shiny_detector(env.console, [&](float error_coefficient) -> bool{
            //  Warning: This callback will be run from a different thread than this function.
            stats.shinies++;
            shiny_coefficient = error_coefficient;
            OverworldShinyDetectedActionOption* action = shiny_action.load(std::memory_order_acquire);
            return on_shiny_callback(env, env.console, *action, error_coefficient);
        });

        int ret = run_until<ProControllerContext>(
            env.console, context,
            [&](ProControllerContext& context){
                ruins_entrance_route(context);

                context.wait_for_all_requests();
                destination_time = current_time();
                shiny_action.store(&SHINY_DETECTED_DESTINATION, std::memory_order_release);

                enter_ruins(context);
            },
            {{shiny_detector}}
        );
        shiny_detector.throw_if_no_sound();
        if (ret == 0 || shiny_detector.last_detection() > destination_time){
            OverworldShinyDetectedActionOption* action = shiny_action.load(std::memory_order_acquire);
            on_shiny_sound(env, env.console, context, *action, shiny_coefficient);
        }
    };

    env.console.log("No shiny detected, returning to Jubilife!");
    goto_camp_from_overworld(env, env.console, context);
    pbf_press_dpad(context, DPAD_RIGHT, 10, 10);
    goto_professor(env.console, context, Camp::MIRELANDS_MIRELANDS);
    from_professor_return_to_jubilife(env, env.console, context);
}


void UnownFinder::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    UnownFinder_Descriptor::Stats& stats = env.current_stats<UnownFinder_Descriptor::Stats>();

    //  Connect the controller.
    pbf_press_button(context, BUTTON_LCLICK, 5, 5);


    bool fresh_from_reset = false;
    while (true){
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS);
        try{
            run_iteration(env, context, fresh_from_reset);
        }catch (OperationFailedException& e){
            stats.errors++;
            e.send_notification(env, NOTIFICATION_ERROR_RECOVERABLE);

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
