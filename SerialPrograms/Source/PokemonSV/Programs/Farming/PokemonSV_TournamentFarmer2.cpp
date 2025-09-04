/*  Tournament Farmer 2
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "PokemonSV/Inference/Battles/PokemonSV_NormalBattleMenus.h"
#include "PokemonSV/Inference/Map/PokemonSV_FastTravelDetector.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/Battles/PokemonSV_SinglesBattler.h"
#include "PokemonSV_TournamentFarmer.h"
#include "PokemonSV_TournamentFarmer2.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;

TournamentFarmer2_Descriptor::TournamentFarmer2_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:TournamentFarmer2",
        STRING_POKEMON + " SV", "Tournament Farmer 2",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/TournamentFarmer2.md",
        "Farm the Academy Ace Tournament for money and prizes. (version 2)",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

struct TournamentFarmer2_Descriptor::Stats : public StatsTracker{
    Stats()
        : tournaments(m_stats["Tournaments"])
        , battles(m_stats["Battles Fought"])
        , wins(m_stats["Wins"])
        , losses(m_stats["Losses"])
//        , money(m_stats["Money Made"])
//        , matches(m_stats["Items Matched"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Tournaments");
        m_display_order.emplace_back("Battles Fought");
        m_display_order.emplace_back("Wins");
        m_display_order.emplace_back("Losses");
//        m_display_order.emplace_back("Money Made");
//        m_display_order.emplace_back("Items Matched");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }
    std::atomic<uint64_t>& tournaments;
    std::atomic<uint64_t>& battles;
    std::atomic<uint64_t>& wins;
    std::atomic<uint64_t>& losses;
//    std::atomic<uint64_t>& money;
//    std::atomic<uint64_t>& matches;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> TournamentFarmer2_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


TournamentFarmer2::StopButton::StopButton()
    : ButtonOption(
      "<b>Stop after Current Tournament:",
      "Stop after current Tournament",
      0, 16
    )
{}
void TournamentFarmer2::StopButton::set_idle(){
    this->set_enabled(false);
    this->set_text("Stop after current Tournament");
}
void TournamentFarmer2::StopButton::set_ready(){
    this->set_enabled(true);
    this->set_text("Stop after current Tournament");
}
void TournamentFarmer2::StopButton::set_pressed(){
    this->set_enabled(false);
    this->set_text("Program will stop after current tournament...");
}


TournamentFarmer2::~TournamentFarmer2(){
    STOP_AFTER_CURRENT.remove_listener(*this);
}
TournamentFarmer2::TournamentFarmer2()
    : NUM_ROUNDS(
        "<b>Number of Tournaments to run:",
        LockMode::UNLOCK_WHILE_RUNNING,
        100, 0
    )
    , SAVE_NUM_ROUNDS(
        "<b>Save every this many tournaments:</b><br>Zero disables saving. Will save win or lose.",
        LockMode::UNLOCK_WHILE_RUNNING,
        1, 0
    )
    , BATTLE_AI(true)
    , GO_HOME_WHEN_DONE(false)
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
    , m_stop_after_current(false)
{
    PA_ADD_OPTION(STOP_AFTER_CURRENT);
    PA_ADD_OPTION(NUM_ROUNDS);
    PA_ADD_OPTION(SAVE_NUM_ROUNDS);
    PA_ADD_OPTION(BATTLE_AI);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);

    STOP_AFTER_CURRENT.set_idle();
    STOP_AFTER_CURRENT.add_listener(*this);
}

void TournamentFarmer2::on_press(){
    global_logger_tagged().log("Stop after current requested...");
    m_stop_after_current.store(true, std::memory_order_relaxed);
    STOP_AFTER_CURRENT.set_pressed();
}



class TournamentFarmer2::ResetOnExit{
public:
    ResetOnExit(StopButton& button)
        : m_button(button)
    {}
    ~ResetOnExit(){
        m_button.set_idle();
    }

private:
    StopButton& m_button;
};



void TournamentFarmer2::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);
    TournamentFarmer2_Descriptor::Stats& stats = env.current_stats<TournamentFarmer2_Descriptor::Stats>();

    m_stop_after_current.store(false, std::memory_order_relaxed);
    STOP_AFTER_CURRENT.set_ready();
    ResetOnExit reset_button_on_exit(STOP_AFTER_CURRENT);


    for (uint32_t c = 0; c < NUM_ROUNDS; c++){
        if (m_stop_after_current.load(std::memory_order_relaxed)){
            break;
        }

        env.log("Tournament loop started.");
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);

        //  Initiate dialog then mash until first battle starts
        {
            AdvanceDialogWatcher advance_detector(COLOR_YELLOW);
            pbf_press_button(context, BUTTON_A, 10, 50);
            pbf_press_button(context, BUTTON_A, 10, 50);
            int ret = wait_until(env.console, context, Milliseconds(7000), { advance_detector });
            if (ret == 0){
                env.log("Dialog detected.");
            }else{
                env.log("Dialog not detected.");
            }
            pbf_mash_button(context, BUTTON_A, 400);
            context.wait_for_all_requests();
        }
        {
            NormalBattleMenuWatcher battle_menu(COLOR_YELLOW);
            int ret = run_until<ProControllerContext>(
                env.console, context,
                [](ProControllerContext& context){
                    pbf_mash_button(context, BUTTON_B, 10000); //it takes a while to load and start
                },
                {battle_menu}
            );
            if (ret != 0){
                stats.errors++;
                env.update_stats();
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "Failed to detect battle start!",
                    env.console
                );
            }
        }
        context.wait_for_all_requests();

        stats.tournaments++;
        env.update_stats();

        bool battle_lost = false;
        for (uint16_t battles = 0; battles < 4; battles++){
            NormalBattleMenuWatcher battle_menu(COLOR_YELLOW);  //  Next battle started
            OverworldWatcher overworld(env.console, COLOR_CYAN);             //  Previous battle was lost
            int ret = run_until<ProControllerContext>(
                env.console, context,
                [](ProControllerContext& context){
                    pbf_mash_button(context, BUTTON_B, 120 * TICKS_PER_SECOND);
                },
                {battle_menu, overworld}
            );
            context.wait_for_all_requests();

            switch (ret){
            case 0:
                env.log("Detected battle menu.");
                run_singles_battle(env, env.console, context, BATTLE_AI, true);
                stats.battles++;
                env.update_stats();
                break;
            case 1:
                env.log("Detected overworld.");
                battle_lost = true;
                stats.losses++;
                env.update_stats();
                break;
            default:
                env.log("Failed to detect battle menu or dialog prompt!");
                stats.errors++;
                env.update_stats();
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "Failed to detect battle menu or dialog prompt!",
                    env.console
                );
            }


            if (battles == 3){
                env.log("Final battle of the tournament complete, checking for overworld.");

                context.wait_for_all_requests();

                /* 
                - mash B to clear dialog until it reaches the overworld.
                - then it looks for the Fast travel icon
                  - if win: Fast Travel will be detected
                  - if lose: will time out.
                */
                ret = run_until<ProControllerContext>(
                    env.console, context,
                    [](ProControllerContext& context){
                        pbf_mash_button(context, BUTTON_B, 120 * TICKS_PER_SECOND);
                    },
                    {overworld} 
                );
                if (ret < 0){
                    OperationFailedException::fire(
                        ErrorReport::SEND_ERROR_REPORT,
                        "Failed to return to overworld after 2 minutes.",
                        env.console
                    );
                }
                context.wait_for_all_requests();

                FastTravelWatcher fast_travel(COLOR_YELLOW, env.console.overlay(), MINIMAP_AREA);
                ret = run_until<ProControllerContext>(
                    env.console, context,
                    [](ProControllerContext& context){
                        pbf_mash_button(context, BUTTON_B, 5 * TICKS_PER_SECOND);
                    },
                    {fast_travel}
                );
                context.wait_for_all_requests();

                switch (ret){
                case 0:
                    env.log("Detected fast travel icon.");
                    env.log("Final battle of the tournament won.");
                    break;
                default:
                    env.log("Final battle of the tournament lost.");
                    battle_lost = true;
                    stats.losses++;
                    env.update_stats();
                    break;
                }
            }


            if (battle_lost){
                return_to_academy_after_loss(env, env.console, context);
                break;
            }
        }

        //  Tournament won
        if (!battle_lost){
            stats.wins++;
            env.update_stats();
        }

        env.log("Tournament loop complete.");

        //Save the game if option is set
        uint16_t num_rounds_temp = SAVE_NUM_ROUNDS;
        if (num_rounds_temp != 0 && ((c + 1) % num_rounds_temp) == 0){
            env.log("Saving game.");
            save_game_from_overworld(env.program_info(), env.console, context);
        }

    }

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}









}
}
}
