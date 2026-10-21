/*  BBQ Solo Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonTools/StartupChecks/StartProgramChecks.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "NintendoSwitch/Programs/DateSpam/NintendoSwitch_HomeToDateTime.h"
#include "Pokemon/Pokemon_Strings.h"
#include "VideoGames/PokemonSV/Programs/PokemonSV_GameEntry.h"
#include "VideoGames/PokemonSV/Programs/PokemonSV_SaveGame.h"
#include "VideoGames/PokemonSV/Programs/PokemonSV_MenuNavigation.h"
#include "VideoGames/PokemonSV/Programs/PokemonSV_WorldNavigation.h"
//#include "VideoGames/PokemonSV/PokemonSV_Settings.h"
#include "VideoGames/PokemonSV/Programs/Farming/PokemonSV_BlueberryQuests.h"
#include "PokemonSV_BBQSoloFarmer.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;
 
BBQSoloFarmer_Descriptor::BBQSoloFarmer_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:BBQSoloFarmer",
        STRING_POKEMON + " SV", "BBQ Farmer",
        "Programs/PokemonSV/BBQSoloFarmer.html",
        "Farm Blueberry Quests in the Terarium for BP.",
        ProgramControllerClass::StandardController_RequiresPrecision,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        PanelDeprecation::NOT_DEPRECATED,
        {
            "PaddleOCRDetection",
        }
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

void run_bbq_rounds(Logger& logger, const BBQOption& options, BBQGameActions& game){
    std::vector<BBQuests> quest_list; //all quests
    std::vector<BBQuests> quests_to_do; //do-able quests
    uint8_t eggs_hatched = 0; //Track eggs
    uint64_t num_completed_quests = 0;

    while (num_completed_quests < options.NUM_QUESTS){
        if (options.OUT_OF_EGGS == BBQOption::OOEggs::Stop && eggs_hatched >= options.NUM_EGGS){
            logger.log("Stop when out of eggs selected. Stopping program.");
            break;
        }

        //Get and reroll quests until we can at least one
        while (quests_to_do.size() < 1){
            quest_list = game.read_quests();
            quests_to_do = game.process_quest_list(quest_list, eggs_hatched);

            //Clear out the regular quest list.
            quest_list.clear();
        }

        for (auto current_quest : quests_to_do){
            //Check if quest was already completed (ex. 500 meters completed while navigating to take a photo)
            quest_list = game.read_quests();
            if (std::find(quest_list.begin(), quest_list.end(), current_quest) != quest_list.end()){
                logger.log("Current quest exists on list. Doing quest.");
                bool questSuccess = game.do_quest(current_quest, eggs_hatched);
                if (questSuccess){
                    logger.log("Quest completed successfully.");
                    game.quest_completed();
                    num_completed_quests++;
                }else{
                    logger.log("Quest did not complete successfully.");
                }
            }else{
                //Note: This doesn't account for case such as "sneak up" being added and then completed alongside the next quest
                logger.log("Current quest does not exist on list. Quest completed at some point.");
                game.quest_completed();
                num_completed_quests++;
            }
            quest_list.clear();
        }
        //Clear out the todo list
        quests_to_do.clear();

        uint64_t temp_save_num_option = options.SAVE_NUM_QUESTS;
        game.end_round(temp_save_num_option != 0 && num_completed_quests % temp_save_num_option == 0);
    }
}


namespace{

class BBQSoloFarmerGameActions : public BBQGameActions{
public:
    BBQSoloFarmerGameActions(
        SingleSwitchProgramEnvironment& env, ProControllerContext& context,
        BBQOption& options, BBQSoloFarmer_Descriptor::Stats& stats,
        EventNotificationOption& status_update
    )
        : m_env(env), m_context(context)
        , m_options(options), m_stats(stats)
        , m_status_update(status_update)
    {}

    virtual std::vector<BBQuests> read_quests() override{
        return PokemonSV::read_quests(m_env.program_info(), m_env.console, m_context, m_options);
    }
    virtual std::vector<BBQuests> process_quest_list(
        const std::vector<BBQuests>& quests, uint8_t eggs_hatched
    ) override{
        return PokemonSV::process_quest_list(m_env.program_info(), m_env.console, m_context, m_options, quests, eggs_hatched);
    }
    virtual bool do_quest(BBQuests quest, uint8_t& eggs_hatched) override{
        return process_and_do_quest(m_env, m_env.console, m_context, m_options, quest, eggs_hatched);
    }
    virtual void quest_completed() override{
        m_stats.questsCompleted++;
        m_env.update_stats();
    }
    virtual void end_round(bool save) override{
        //Fix the time to prevent running out of years
        pbf_wait(m_context, 2000ms);
        m_context.wait_for_all_requests();
        go_home(m_env.console, m_context);
        home_to_date_time(m_env.console, m_context, false);
        pbf_press_button(m_context, BUTTON_A, 160ms, 840ms);
        pbf_press_button(m_context, BUTTON_A, 160ms, 840ms);
        pbf_press_button(m_context, BUTTON_HOME, 160ms, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY0);
        resume_game_from_home(m_env.console, m_context);

        if (save){
            m_env.log("Saving and resetting.");
            save_game_from_overworld(m_env.program_info(), m_env.console, m_context);
            reset_game(m_env.program_info(), m_env.console, m_context);
            m_stats.saves++;
        }

        m_env.update_stats();
        send_program_status_notification(m_env, m_status_update);
    }

private:
    SingleSwitchProgramEnvironment& m_env;
    ProControllerContext& m_context;
    BBQOption& m_options;
    BBQSoloFarmer_Descriptor::Stats& m_stats;
    EventNotificationOption& m_status_update;
};

}

void BBQSoloFarmer::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    StartProgramChecks::check_performance_class_wired_or_wireless(context);
    assert_16_9_720p_min(env.logger(), env.console);

    BBQSoloFarmer_Descriptor::Stats& stats = env.current_stats<BBQSoloFarmer_Descriptor::Stats>();

    //Make sure console type is set
    if (env.console.state().console_type() == ConsoleType::Unknown){
        throw UserSetupError(env.console, "Console Type (Switch 1 or 2) must be specified.");
    }

    //  Connect the controller.
    require_player(env.console, context, BUTTON_LCLICK);
    
    //Fly to plaza
    open_map_from_overworld(env.program_info(), env.console, context);
    fly_to_overworld_from_map(env.program_info(), env.console, context);

    //Test a specific quest
    /*
    uint8_t eggs_hatched = 0;
    BBQuests test_quest = BBQuests::catch_water;
    bool questTest = process_and_do_quest(env, env.console, context, BBQ_OPTIONS, test_quest, eggs_hatched);
    if (questTest){
        env.log("Finished quest.");
    }
    */

    BBQSoloFarmerGameActions game(env, context, BBQ_OPTIONS, stats, NOTIFICATION_STATUS_UPDATE);
    run_bbq_rounds(env.logger(), BBQ_OPTIONS, game);
    env.update_stats();

    if (BBQ_OPTIONS.FIX_TIME_WHEN_DONE){
        go_home(env.console, context);
        home_to_date_time(env.console, context, false);
        pbf_press_button(context, BUTTON_A, 160ms, 840ms);
        pbf_press_button(context, BUTTON_A, 160ms, 840ms);
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
