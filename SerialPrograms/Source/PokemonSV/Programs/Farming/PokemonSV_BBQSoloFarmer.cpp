/*  BBQ Solo Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonTools/StartupChecks/StartProgramChecks.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "NintendoSwitch/Programs/DateSpam/NintendoSwitch_HomeToDateTime.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "PokemonSV/Programs/PokemonSV_MenuNavigation.h"
#include "PokemonSV/Programs/PokemonSV_WorldNavigation.h"
//#include "PokemonSV/PokemonSV_Settings.h"
#include "PokemonSV/Programs/Farming/PokemonSV_BlueberryQuests.h"
#include "PokemonSV_BBQSoloFarmer.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;
 
BBQSoloFarmer_Descriptor::BBQSoloFarmer_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:BBQSoloFarmer",
        STRING_POKEMON + " SV", "BBQ Farmer",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/BBQSoloFarmer.md",
        "Farm Blueberry Quests in the Terarium for BP.",
        ProgramControllerClass::StandardController_RequiresPrecision,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        {}
    )
{}
struct BBQSoloFarmer_Descriptor::Stats : public StatsTracker{
    Stats()
        : questsCompleted(m_stats["Quests Completed"])
        , saves(m_stats["Saves"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Quests Completed");
        m_display_order.emplace_back("Saves");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }
    std::atomic<uint64_t>& questsCompleted;
    std::atomic<uint64_t>& saves;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> BBQSoloFarmer_Descriptor::make_stats() const{
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

void BBQSoloFarmer::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    StartProgramChecks::check_performance_class_wired_or_wireless(context);
    assert_16_9_720p_min(env.logger(), env.console);

    BBQSoloFarmer_Descriptor::Stats& stats = env.current_stats<BBQSoloFarmer_Descriptor::Stats>();

    //Make sure console type is set
    if (env.console.state().console_type() == ConsoleType::Unknown) {
        throw UserSetupError(env.console, "Console Type (Switch 1 or 2) must be specified.");
    }
    
    //Fly to plaza
    open_map_from_overworld(env.program_info(), env.console, context);
    fly_to_overworld_from_map(env.program_info(), env.console, context);

    std::vector<BBQuests> quest_list; //all quests
    std::vector<BBQuests> quests_to_do; //do-able quests
    uint8_t eggs_hatched = 0; //Track eggs
    uint64_t num_completed_quests = 0;

    //Test a specific quest
    /*
    BBQuests test_quest = BBQuests::catch_water;
    bool questTest = process_and_do_quest(env, env.console, context, BBQ_OPTIONS, test_quest, eggs_hatched);
    if (questTest){
        env.log("Finished quest.");
    }
    */

    while (num_completed_quests < BBQ_OPTIONS.NUM_QUESTS){
        if (BBQ_OPTIONS.OUT_OF_EGGS == BBQOption::OOEggs::Stop && eggs_hatched >= BBQ_OPTIONS.NUM_EGGS){
            env.log("Stop when out of eggs selected. Stopping program.");
            break;
        }

        //Get and reroll quests until we can at least one
        while (quests_to_do.size() < 1){
            quest_list = read_quests(env.program_info(), env.console, context, BBQ_OPTIONS);
            quests_to_do = process_quest_list(env.program_info(), env.console, context, BBQ_OPTIONS, quest_list, eggs_hatched);

            //Clear out the regular quest list.
            quest_list.clear();
        }

        for (auto current_quest : quests_to_do){
            //Check if quest was already completed (ex. 500 meters completed while navigating to take a photo)
            quest_list = read_quests(env.program_info(), env.console, context, BBQ_OPTIONS);
            if (std::find(quest_list.begin(), quest_list.end(), current_quest) != quest_list.end()){
                env.log("Current quest exists on list. Doing quest.");
                bool questSuccess = process_and_do_quest(env, env.console, context, BBQ_OPTIONS, current_quest, eggs_hatched);
                if (questSuccess){
                    env.log("Quest completed successfully.");
                    stats.questsCompleted++;
                    env.update_stats();
                    num_completed_quests++;
                }else{
                    env.log("Quest did not complete successfully.");
                }
            }else{
                //Note: This doesn't account for case such as "sneak up" being added and then completed alongside the next quest
                env.log("Current quest does not exist on list. Quest completed at some point.");
                stats.questsCompleted++;
                env.update_stats();
                num_completed_quests++;
            }
            quest_list.clear();
        }
        //Clear out the todo list
        quests_to_do.clear();

        //Fix the time to prevent running out of years
        pbf_wait(context, 250);
        context.wait_for_all_requests();
        go_home(env.console, context);
        home_to_date_time(env.console, context, false);
        pbf_press_button(context, BUTTON_A, 20, 105);
        pbf_press_button(context, BUTTON_A, 20, 105);
        pbf_press_button(context, BUTTON_HOME, 160ms, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY0);
        resume_game_from_home(env.console, context);

        uint64_t temp_save_num_option = BBQ_OPTIONS.SAVE_NUM_QUESTS;
        if (temp_save_num_option != 0 && num_completed_quests % temp_save_num_option == 0){
            env.log("Saving and resetting.");
            save_game_from_overworld(env.program_info(), env.console, context);
            reset_game(env.program_info(), env.console, context);
            stats.saves++;
        }

        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);
    }
    env.update_stats();

    if (BBQ_OPTIONS.FIX_TIME_WHEN_DONE){
        go_home(env.console, context);
        home_to_date_time(env.console, context, false);
        pbf_press_button(context, BUTTON_A, 20, 105);
        pbf_press_button(context, BUTTON_A, 20, 105);
        pbf_press_button(context, BUTTON_HOME, 160ms, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY0);
        resume_game_from_home(env.console, context);

        open_map_from_overworld(env.program_info(), env.console, context);
        fly_to_overworld_from_map(env.program_info(), env.console, context);
    }

    GO_HOME_WHEN_DONE.run_end_of_program(context);
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}
    
}
}
}
