/*  BBQ Solo Farmer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/Tools/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/PokemonSV_Navigation.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "PokemonSV/Inference/Battles/PokemonSV_NormalBattleMenus.h"
#include "PokemonSV/Programs/PokemonSV_BlueberryQuests.h"
#include "PokemonSV_BBQSoloFarmer.h"

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonSV {

using namespace Pokemon;
 
BBQSoloFarmer_Descriptor::BBQSoloFarmer_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:BBQSoloFarmer",
        STRING_POKEMON + " SV", "BBQ Farmer - Solo",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/BBQSoloFarmer.md",
        "Farm Blueberry Quests in the Terarium.",
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}
struct BBQSoloFarmer_Descriptor::Stats : public StatsTracker {
    Stats()
        : blueQuests(m_stats["Blue Quests"])
        , redQuests(m_stats["Red Quests"])
        , earnedBP(m_stats["BP Earned"])
        , rerolls(m_stats["Quest Rerolls"])
        , saves(m_stats["Saves"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Blue Quests");
        m_display_order.emplace_back("Red Quests");
        m_display_order.emplace_back("BP Earned");
        m_display_order.emplace_back("Quest Rerolls");
        m_display_order.emplace_back("Saves");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }
    std::atomic<uint64_t>& blueQuests;
    std::atomic<uint64_t>& redQuests;
    std::atomic<uint64_t>& earnedBP;
    std::atomic<uint64_t>& rerolls;
    std::atomic<uint64_t>& saves;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> BBQSoloFarmer_Descriptor::make_stats() const {
    return std::unique_ptr<StatsTracker>(new Stats());
}
BBQSoloFarmer::BBQSoloFarmer()
    : GO_HOME_WHEN_DONE(true)
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        & NOTIFICATION_PROGRAM_FINISH,
        & NOTIFICATION_ERROR_FATAL,
        })
{
    PA_ADD_OPTION(BBQ_OPTIONS);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void BBQSoloFarmer::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) {
    assert_16_9_720p_min(env.logger(), env.console);
    BBQSoloFarmer_Descriptor::Stats& stats = env.current_stats<BBQSoloFarmer_Descriptor::Stats>();

    /*
    start at Central Plaza - we return here after every task (switch map and then switch back to center the map)
    must have flying unlocked
    must have certain fly points unlocked
    must have completed the DLC storyline (caught turtle), this is needed for tera orb recharge and flying.
    open BP quests panel and read first task. perform the task.
    keep track of how much BP earned
    for quests that cannot be done, reroll. track the BP cost of rerolling.
    use the fixed and tera spawns for photographs

    smoke ball or flying pokemon required in slot 1 in case of arena trap
    handle out of bp rerolls?
    */

    std::vector<std::string> quest_list; //all quests
    std::vector<std::string> quests_to_do; //do-able quests
    int eggs_hatched = 0; //Track eggs

    //Get and reroll quests until we can at least one
    while (quests_to_do.size() < 1) {
        read_quests(env.program_info(), env.console, context, BBQ_OPTIONS, quest_list);
        process_quest_list(env.program_info(), env.console, context, BBQ_OPTIONS, quest_list, quests_to_do, eggs_hatched);
        //Clear out the regular quest list.
        quest_list.clear();
    }

    env.log("Quests to do: ");
    for (auto n : quests_to_do) {
        env.log(n);
    }



    //return_to_plaza(env.program_info(), env.console, context);

    /*
    for (uint64_t i = 0; i < NUM_QUESTS; i++) {
        //Open the BP menu and read the quest

        //manage this in the quest function
        //Execute quest, return true if successful. false if not.
        //reattempt quest if failed

        //Return to central plaza
        
        
        
        if (SAVE_NUM_QUESTS != 0 && i % SAVE_NUM_ROUNDS == 0) {
            env.log("Saving and resetting.");
            save_game_from_overworld(env.program_info(), env.console, context);
            reset_game(env.program_info(), env.console, context);
            stats.m_saves++;
        }
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);
    }
    */
    stats.saves++;

    env.update_stats();
    GO_HOME_WHEN_DONE.run_end_of_program(context);
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}
    
}
}
}
