/*  Money Farmer (Highlands)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/StatsTracking.h"
//#include "CommonFramework/Tools/InterruptableCommands.h"
//#include "CommonFramework/Tools/SuperControlSession.h"
#include "CommonFramework/InferenceInfra/VisualInferenceRoutines.h"
#include "CommonFramework/Inference/BlackScreenDetector.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonLA/PokemonLA_Settings.h"
#include "PokemonLA/Inference/PokemonLA_MapDetector.h"
#include "PokemonLA/Inference/PokemonLA_DialogDetector.h"
#include "PokemonLA/Inference/PokemonLA_OverworldDetector.h"
#include "PokemonLA/Programs/PokemonLA_GameEntry.h"
#include "PokemonLA/Programs/PokemonLA_RegionNavigation.h"
#include "PokemonLA_NuggetFarmerHighlands.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


NuggetFarmerHighlands_Descriptor::NuggetFarmerHighlands_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonLA:NuggetFarmerHighlands",
        STRING_POKEMON + " LA", "Nugget Farmer (Highlands)",
        "ComputerControl/blob/master/Wiki/Programs/PokemonLA/NuggetFarmerHighlands.md",
        "Farm nuggets off the Miss Fortune sisters in the Coronet Highlands.",
        FeedbackType::REQUIRED, false,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}


MoneyFarmerHighlands::MoneyFarmerHighlands(const NuggetFarmerHighlands_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , NOTIFICATION_STATUS("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS,
//        &SHINY_DETECTED.NOTIFICATIONS,
//        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
//    PA_ADD_OPTION(SHINY_DETECTED);
    PA_ADD_OPTION(NOTIFICATIONS);
}



class MoneyFarmerHighlands::Stats : public StatsTracker{
public:
    Stats()
        : attempts(m_stats["Attempts"])
        , errors(m_stats["Errors"])
        , charm(m_stats["Charm"])
        , coin(m_stats["Coin"])
    {
        m_display_order.emplace_back("Attempts");
        m_display_order.emplace_back("Errors", true);
        m_display_order.emplace_back("Charm", true);
        m_display_order.emplace_back("Coin", true);
    }

    std::atomic<uint64_t>& attempts;
    std::atomic<uint64_t>& errors;
    std::atomic<uint64_t>& charm;
    std::atomic<uint64_t>& coin;
};

std::unique_ptr<StatsTracker> MoneyFarmerHighlands::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}




void mash_A_until_end_of_battle(ProgramEnvironment& env, ConsoleHandle& console){
    OverworldDetector detector(console, console);
    int ret = run_until(
        env, console,
        [](const BotBaseContext& context){
            pbf_mash_button(context, BUTTON_A, 120 * TICKS_PER_SECOND);
        },
        { &detector }
    );
    if (ret < 0){
        throw OperationFailedException(console, "Failed to return to overworld after 2 minutes.");
    }
    console.log("Returned to overworld.");
}


#if 0
class MoneyFarmerHighlands::RunRoute : public SuperControlSession{
public:
    RunRoute(ProgramEnvironment& env, ConsoleHandle& console)
        : SuperControlSession(env, console)
        , m_dialog_detector(console, console, false)
    {
        *this += m_dialog_detector;
        register_state_command((size_t)State::NOT_STARTED, [=](){
            m_active_command->dispatch([=](const BotBaseContext& context){
                pbf_move_left_joystick(context, 0, 212, 50, 0);
                pbf_press_button(context, BUTTON_B, 495, 80);

                pbf_move_left_joystick(context, 224, 0, 50, 0);
//                    pbf_press_button(context, BUTTON_B, 350, 80);
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
                pbf_press_button(context, BUTTON_B, 270, 0);

                pbf_move_left_joystick(context, 64, 255, 50, 0);
                pbf_press_button(context, BUTTON_B, 150, 250);

//                pbf_move_right_joystick(context, 0, 128, 200, 125);
            });
            return false;
        });
    }

    virtual bool run_state(AsyncCommandSession& commands, WallClock timestamp) override{
        if (last_state() == (size_t)State::NOT_STARTED){
            return run_state_action((size_t)State::RUNNING);
        }
        if (m_dialog_detector.detected()){
            return true;
        }
        return !m_active_command->command_is_running();
    }

private:
    enum class State{
        NOT_STARTED,
        RUNNING,
    };
    DialogDetector m_dialog_detector;
};
#endif



void MoneyFarmerHighlands::run_iteration(SingleSwitchProgramEnvironment& env){
    Stats& stats = env.stats<Stats>();

    //  Go to Coronet Highlands Mountain camp.
    goto_camp_from_jubilife(env, env.console, Camp::HIGHLANDS_MOUNTAIN);

    stats.attempts++;

    //  Switch to Wrydeer.
    bool error = true;
    MountDetector mount_detector;
    for (size_t c = 0; c < 10; c++){
        MountState mount = mount_detector.detect(env.console.video().snapshot());
        if (mount == MountState::WYRDEER_OFF){
            pbf_press_button(env.console, BUTTON_PLUS, 20, 105);
            error = false;
            break;
        }
        if (mount == MountState::WYRDEER_ON){
            pbf_wait(env.console, 5 * TICKS_PER_SECOND);
            error = false;
            break;
        }
        pbf_press_dpad(env.console, DPAD_LEFT, 20, 50);
        env.console.botbase().wait_for_all_requests();
    }
    if (error){
        throw OperationFailedException(env.console, "Unable to find Wyrdeer after 10 attempts.");
    }


    bool success = false;


    env.console.log("Traveling to Charm's location...");
    {
        DialogDetector dialog_detector(env.console, env.console, true);
        //  TODO: Add shiny sound detector.
        int ret = run_until(
            env, env.console,
            [](const BotBaseContext& context){
                pbf_move_left_joystick(context, 0, 212, 50, 0);
                pbf_press_button(context, BUTTON_B, 495, 80);

                pbf_move_left_joystick(context, 224, 0, 50, 0);
//                    pbf_press_button(context, BUTTON_B, 350, 80);
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
                pbf_press_button(context, BUTTON_B, 270, 0);

                pbf_move_left_joystick(context, 64, 255, 50, 0);
                pbf_press_button(context, BUTTON_B, 150, 250);

//                pbf_move_right_joystick(context, 0, 128, 200, 125);

            },
            { &dialog_detector }
        );
        if (ret >= 0){
            env.console.log("Found Charm!", COLOR_BLUE);
            stats.charm++;
            mash_A_until_end_of_battle(env, env.console);
            env.console.log("Battle succeeded!", COLOR_BLUE);
            success = true;
        }
    }


#if 0
    env.console.log("Traveling to Coin's location...");
    {
        DialogDetector dialog_detector;

    }
#endif


    env.console.log("Returning to Jubilife...");
    goto_camp_from_overworld(env, env.console);
    goto_professor(env.console, Camp::HIGHLANDS_HIGHLANDS);
    from_professor_return_to_jubilife(env, env.console);

    if (success){
        save_game_from_overworld(env, env.console);
    }
}



void MoneyFarmerHighlands::program(SingleSwitchProgramEnvironment& env){
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
            pbf_press_button(env.console, BUTTON_HOME, 20, GameSettings::instance().GAME_TO_HOME_DELAY);
            reset_game_from_home(env, env.console, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
        }
    }

}





}
}
}
