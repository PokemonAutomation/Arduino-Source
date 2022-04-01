/*  Post MMO Spawn Reset
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonLA/PokemonLA_Settings.h"
#include "PokemonLA/Inference/PokemonLA_MapDetector.h"
#include "PokemonLA/Inference/PokemonLA_DialogDetector.h"
#include "PokemonLA/Inference/PokemonLA_OverworldDetector.h"
#include "PokemonLA/Inference/PokemonLA_ShinySoundDetector.h"
#include "PokemonLA/Programs/PokemonLA_GameEntry.h"
#include "PokemonLA/Programs/PokemonLA_RegionNavigation.h"
#include "PokemonLA_PostMMOSpawnReset.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


PostMMOSpawnReset_Descriptor::PostMMOSpawnReset_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonLA:PostMMOSpawnReset",
        STRING_POKEMON + " LA", "Post-MMO Spawn Reset",
        "ComputerControl/blob/master/Wiki/Programs/PokemonLA/PostMMOSpawnReset.md",
        "Constantly reset the spawn after MMO finishes.",
        FeedbackType::REQUIRED, false,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}


PostMMOSpawnReset::PostMMOSpawnReset(const PostMMOSpawnReset_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , TURN_DURATION(
        "<b>Camera Turn:</b><br>How many ticks to turn the camera. <br>Positive values for right turns. Negative values for left turns.",
        "0"
    )
    , FORWARD_DURATION(
        "<b>Move Forward:</b><br>After turning the camera, how many ticks to move forward.",
        "0"
    )
    , WAIT_DURATION(
        "<b>Wait Time:</b><br> Wait time after movement.",
        "3 * TICKS_PER_SECOND"
    )
    , NOTIFICATION_STATUS("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS,
        &SHINY_DETECTED.NOTIFICATIONS,
        &NOTIFICATION_PROGRAM_FINISH,
//        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(TURN_DURATION);
    PA_ADD_OPTION(FORWARD_DURATION);
    PA_ADD_OPTION(WAIT_DURATION);
    PA_ADD_OPTION(SHINY_DETECTED);
    PA_ADD_OPTION(NOTIFICATIONS);
}



class PostMMOSpawnReset::Stats : public StatsTracker, public ShinyStatIncrementer{
public:
    Stats()
        : attempts(m_stats["Attempts"])
        , errors(m_stats["Errors"])
        , shinies(m_stats["Shinies"])
    {
        m_display_order.emplace_back("Attempts");
        m_display_order.emplace_back("Errors", true);
        m_display_order.emplace_back("Shinies", true);
    }
    virtual void add_shiny() override{
        shinies++;
    }

    std::atomic<uint64_t>& attempts;
    std::atomic<uint64_t>& errors;
    std::atomic<uint64_t>& shinies;
};

std::unique_ptr<StatsTracker> PostMMOSpawnReset::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


void PostMMOSpawnReset::run_iteration(SingleSwitchProgramEnvironment& env){
    Stats& stats = env.stats<Stats>();

    // From game to Switch Home
    pbf_press_button(env.console, BUTTON_HOME, 20, GameSettings::instance().GAME_TO_HOME_DELAY);
    // Restart the game and go to game menu (where "Press A" is shown to enter the game)
    switch_home_to_gamemenu(env, env.console, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
    {
        ShinySoundDetector shiny_detector(env.console, SHINY_DETECTED.stop_on_shiny());
        run_until(
            env, env.console,
            [this, &env](const BotBaseContext& context){
                // TODO: gamemenu_to_ingame has wait_until(). Nesting wait_until() into run_until() is not the intended usage of run_until().
                // When outer run_until() wants to break out, the inner wait_until() may not be able to do that until itself is finished.
                // But this is fine in the current use case, as we are OK with the shiny sound detector stopping the program a little late.
                gamemenu_to_ingame(env, env.console, GameSettings::instance().ENTER_GAME_MASH, GameSettings::instance().ENTER_GAME_WAIT);
                env.console.log("Entered game! Checking shiny sound...");

                // forward portion
                if (TURN_DURATION > 0){
                    pbf_move_right_joystick(context, 255, 128, uint16_t(TURN_DURATION), 0);
                } else if (TURN_DURATION < 0){
                    pbf_move_right_joystick(context, 0, 128, uint16_t(-TURN_DURATION), 0);
                }
                
                pbf_controller_state(context, BUTTON_LCLICK, DPAD_NONE, 128, 0, 128, 128, FORWARD_DURATION);

                pbf_wait(context, WAIT_DURATION);

                context.wait_for_all_requests();
            },
            { &shiny_detector });
        if (shiny_detector.detected()){
           stats.shinies++;
           on_shiny_sound(env, env.console, SHINY_DETECTED, shiny_detector.results());
        }
    }

    stats.attempts++;
    env.console.log("No shiny detected, restarting the game!");
}


void PostMMOSpawnReset::program(SingleSwitchProgramEnvironment& env){
    Stats& stats = env.stats<Stats>();

    //  Connect the controller.
    pbf_press_button(env.console, BUTTON_LCLICK, 5, 5);

    while (true){
        env.update_stats();
        send_program_status_notification(
            env.logger(), NOTIFICATION_STATUS,
            env.program_info(),
            "",
            stats.to_str()
        );
        try{
            run_iteration(env);
        }catch (OperationFailedException&){
            stats.errors++;
            // run_iteration() restarts the game first then listens to shiny sound.
            // If there is any error generated when the game is running and is caught here,
            // we just do nothing to handle the error as in the next iteration of run_iteration()
            // the game will be immediately restarted.
        }
    }

    env.update_stats();
    send_program_finished_notification(
        env.logger(), NOTIFICATION_PROGRAM_FINISH,
        env.program_info(),
        "",
        stats.to_str()
    );
}



}
}
}
