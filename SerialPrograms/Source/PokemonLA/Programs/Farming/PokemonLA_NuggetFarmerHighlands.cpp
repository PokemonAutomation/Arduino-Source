/*  Money Farmer (Highlands)
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
#include "PokemonLA/Inference/PokemonLA_DialogDetector.h"
#include "PokemonLA/Inference/Sounds/PokemonLA_ShinySoundDetector.h"
#include "PokemonLA/Programs/PokemonLA_BattleRoutines.h"
#include "PokemonLA/Programs/PokemonLA_GameEntry.h"
#include "PokemonLA/Programs/PokemonLA_GameSave.h"
#include "PokemonLA/Programs/PokemonLA_MountChange.h"
#include "PokemonLA/Programs/PokemonLA_RegionNavigation.h"
#include "PokemonLA_NuggetFarmerHighlands.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{
    using namespace Pokemon;


NuggetFarmerHighlands_Descriptor::NuggetFarmerHighlands_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLA:NuggetFarmerHighlands",
        STRING_POKEMON + " LA", "Nugget Farmer (Highlands)",
        "ComputerControl/blob/master/Wiki/Programs/PokemonLA/NuggetFarmerHighlands.md",
        "Farm nuggets off the Miss Fortune sisters in the Coronet Highlands.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::VIDEO_AUDIO,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
class NuggetFarmerHighlands_Descriptor::Stats : public StatsTracker, public ShinyStatIncrementer{
public:
    Stats()
        : attempts(m_stats["Attempts"])
        , errors(m_stats["Errors"])
        , charm(m_stats["Charm"])
        , coin(m_stats["Coin"])
        , shinies(m_stats["Shinies"])
    {
        m_display_order.emplace_back("Attempts");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Charm", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Coin", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Shinies", HIDDEN_IF_ZERO);
    }
    virtual void add_shiny() override{
        shinies++;
    }

    std::atomic<uint64_t>& attempts;
    std::atomic<uint64_t>& errors;
    std::atomic<uint64_t>& charm;
    std::atomic<uint64_t>& coin;
    std::atomic<uint64_t>& shinies;
};
std::unique_ptr<StatsTracker> NuggetFarmerHighlands_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


NuggetFarmerHighlands::NuggetFarmerHighlands()
    : SHINY_DETECTED("Shiny Detected Action", "", "2000 ms")
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
    PA_ADD_OPTION(SHINY_DETECTED);
    PA_ADD_OPTION(NOTIFICATIONS);
}



bool NuggetFarmerHighlands::run_iteration(
    SingleSwitchProgramEnvironment& env, ProControllerContext& context,
    bool fresh_from_reset
){
    NuggetFarmerHighlands_Descriptor::Stats& stats = env.current_stats<NuggetFarmerHighlands_Descriptor::Stats>();

    //  Go to Coronet Highlands Mountain camp.
    goto_camp_from_jubilife(
        env, env.console, context,
        TravelLocations::instance().Highlands_Mountain,
        fresh_from_reset
    );

    stats.attempts++;

    change_mount(env.console, context, MountState::WYRDEER_ON);


    bool success = false;


    env.console.log("Traveling to Charm's location...");
    {
        DialogSurpriseDetector dialog_detector(env.console, env.console, true);

        float shiny_coefficient = 1.0;
        ShinySoundDetector shiny_detector(env.console, [&](float error_coefficient) -> bool{
            //  Warning: This callback will be run from a different thread than this function.
            stats.shinies++;
            shiny_coefficient = error_coefficient;
            return on_shiny_callback(env, env.console, SHINY_DETECTED, error_coefficient);
        });

        int ret = run_until<ProControllerContext>(
            env.console, context,
            [](ProControllerContext& context){
                pbf_move_left_joystick(context, 0, 212, 50, 0);
                pbf_press_button(context, BUTTON_B, 492, 80);

                pbf_move_left_joystick(context, 224, 0, 50, 0);
//                pbf_press_button(context, BUTTON_B, 350, 80);
                pbf_press_button(context, BUTTON_B, 80, 0);
                for (size_t c = 0; c < 7; c++){
                    pbf_press_button(context, BUTTON_A | BUTTON_B, 5, 0);
                    pbf_press_button(context, BUTTON_B, 5, 0);
                }
                pbf_press_button(context, BUTTON_B, 200, 80);
                pbf_wait(context, 80);

                pbf_move_left_joystick(context, 0, 64, 50, 0);
                pbf_press_button(context, BUTTON_B, 250, 80);

                pbf_move_left_joystick(context, 0, 48, 50, 0);
                pbf_press_button(context, BUTTON_B, 250, 0);

                pbf_move_left_joystick(context, 64, 255, 50, 0);
                pbf_press_button(context, BUTTON_B, 150, 250);

//                pbf_move_right_joystick(context, 0, 128, 200, 125);

            },
            {
                {dialog_detector},
                {shiny_detector},
            }
        );
        switch (ret){
        case 0:
            env.console.log("Found Charm!", COLOR_BLUE);
            stats.charm++;
            mash_A_until_end_of_battle(env.console, context);
            env.console.log("Battle succeeded!", COLOR_BLUE);
            success = true;
            break;
        case 1:
            on_shiny_sound(env, env.console, context, SHINY_DETECTED, shiny_coefficient);
            break;
        }
    }


#if 0
    env.console.log("Traveling to Coin's location...");
    {

    }
#endif


    env.console.log("Returning to Jubilife...");


    {
        float shiny_coefficient = 1.0;
        ShinySoundDetector shiny_detector(env.console, [&](float error_coefficient) -> bool{
            //  Warning: This callback will be run from a different thread than this function.
            stats.shinies++;
            shiny_coefficient = error_coefficient;
            return on_shiny_callback(env, env.console, SHINY_DETECTED, error_coefficient);
        });

        int ret = run_until<ProControllerContext>(env.console, context,
            [&env](ProControllerContext& context){
                goto_camp_from_overworld(env, env.console, context);
                goto_professor(env.console, context, Camp::HIGHLANDS_HIGHLANDS);
            },
            {{shiny_detector}}
        );
        shiny_detector.throw_if_no_sound();
        if (ret == 0){
            on_shiny_sound(env, env.console, context, SHINY_DETECTED, shiny_coefficient);
        }
    }


    from_professor_return_to_jubilife(env, env.console, context);

    if (success){
        save_game_from_overworld(env, env.console, context);
    }

    return false;
}



void NuggetFarmerHighlands::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    NuggetFarmerHighlands_Descriptor::Stats& stats = env.current_stats<NuggetFarmerHighlands_Descriptor::Stats>();

    //  Connect the controller.
    pbf_press_button(context, BUTTON_LCLICK, 5, 5);

    // Put a save here so that when the program reloads from error it won't break.
    save_game_from_overworld(env, env.console, context);

    bool fresh_from_reset = false;
    while (true){
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS);
        try{
            if (run_iteration(env, context, fresh_from_reset)){
                break;
            }
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
