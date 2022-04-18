/*  Tree Shiny Hunting
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/Inference/BlackScreenDetector.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonLA/PokemonLA_Settings.h"
#include "PokemonLA/Inference/Battles/PokemonLA_BattleMenuDetector.h"
#include "PokemonLA/Inference/PokemonLA_StatusInfoScreenDetector.h"
#include "PokemonLA/Inference/Objects/PokemonLA_ButtonDetector.h"
#include "PokemonLA/Inference/PokemonLA_MapDetector.h"
#include "PokemonLA/Inference/Sounds/PokemonLA_ShinySoundDetector.h"
#include "PokemonLA/Inference/PokemonLA_UnderAttackDetector.h"
#include "PokemonLA/Programs/PokemonLA_MountChange.h"
#include "PokemonLA/Programs/PokemonLA_GameEntry.h"
#include "PokemonLA/Programs/PokemonLA_RegionNavigation.h"
#include "PokemonLA/Inference/Objects/PokemonLA_DialogueEllipseDetector.h"
#include "PokemonLA/Programs/ShinyHunting/PokemonLA_TreeShinyHunting.h"


namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


TreeShinyHunting_Descriptor::TreeShinyHunting_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonLA:Tree Shiny Hunter",
        STRING_POKEMON + " LA", "Tree Shiny Hunter",
        "ComputerControl/blob/master/Wiki/Programs/PokemonLA/TreeShinyHunter.md",
        "Check nearby trees for a possible Shiny, Alpha or Alpha Shiny Burmy",
        FeedbackType::REQUIRED, false,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}


TreeShinyHunting::TreeShinyHunting(const TreeShinyHunting_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , MATCH_DETECTED_OPTIONS(
      "Match Action",
      "What to do when the pokemon matches the expectations",
      "0 * TICKS_PER_SECOND")
    , STOP_ON(
        "<b>Stop On:</b>",
        {
        "Shiny",
        "Alpha",
        "Shiny & Alpha",
        "Stop on any non regular"
        },
        0
    )
    , NOTIFICATION_STATUS("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS,
        &MATCH_DETECTED_OPTIONS.NOTIFICATIONS,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(MATCH_DETECTED_OPTIONS);
    PA_ADD_OPTION(STOP_ON);
    PA_ADD_OPTION(NOTIFICATIONS);
}


class TreeShinyHunting::Stats : public StatsTracker, public ShinyStatIncrementer{
public:
    Stats()
        : attempts(m_stats["Attempts"])
        , errors(m_stats["Errors"])
        , found(m_stats["Found"])
        , shinies(m_stats["Shinies"])
        , alphas(m_stats["Alphas"])
    {
        m_display_order.emplace_back("Attempts");
        m_display_order.emplace_back("Errors", true);
        m_display_order.emplace_back("Found", true);
        m_display_order.emplace_back("Shinies", true);
        m_display_order.emplace_back("Alphas", true);
    }

    virtual void add_shiny() override{
        shinies++;
    }

    std::atomic<uint64_t>& attempts;
    std::atomic<uint64_t>& errors;
    std::atomic<uint64_t>& found;
    std::atomic<uint64_t>& shinies;
    std::atomic<uint64_t>& alphas;

};

std::unique_ptr<StatsTracker> TreeShinyHunting::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


bool TreeShinyHunting::check_tree(SingleSwitchProgramEnvironment& env, BotBaseContext& context, int16_t expected){

    Stats& stats = env.stats<Stats>();

    pbf_press_button(context, BUTTON_ZR, (0.5 * TICKS_PER_SECOND), 20); //throw pokemon
    pbf_wait(context, (4 * TICKS_PER_SECOND));
    context.wait_for_all_requests();

    MountDetector mount_detector;
    MountState mount = mount_detector.detect(env.console.video().snapshot());

    if (mount != MountState::NOTHING){
       env.console.log("Battle not found. Tree might be empty.");
       return false;
    }

    env.console.log("Battle found!");

    BattleMenuDetector battle_menu_detector(env.console, env.console, true);
    wait_until(
       env.console, context, std::chrono::seconds(10),
       {
           {battle_menu_detector}
       }
    );

    pbf_wait(context, (1 * TICKS_PER_SECOND));
    pbf_press_button(context, BUTTON_PLUS, 20, 20);
    pbf_wait(context, (1 * TICKS_PER_SECOND));
    pbf_press_button(context, BUTTON_R, 20, 20);
    pbf_wait(context, (1 * TICKS_PER_SECOND));

    context.wait_for_all_requests();

    QImage infoScreen = env.console.video().snapshot();
    StatusInfoScreenDetector detector;

    detector.process_frame(infoScreen, std::chrono::system_clock::now());

    int16_t ret = detector.detected();

    env.console.log("RETURN: " + std::to_string(ret));
    env.console.log("EXPECT: " + std::to_string(expected));

    context.wait_for_all_requests();

    if(ret == 0){
       env.console.log("Normie in the tree -_-");
       stats.found++;
       exit_battle(context);
       return false;
    }

    switch (ret) {
        case 1:
            env.console.log("Found SHINY");
            stats.shinies++;
            break;

        case 2:
            env.console.log("FOUND ALPHA");
            stats.alphas++;
            break;

        case 3:
             env.console.log("FOUND SHINY ALPHA");
             stats.shinies++;
             stats.alphas++;
            break;
    }

    on_match_found(env, env.console, context, MATCH_DETECTED_OPTIONS, (ret == expected || expected == 4));

    exit_battle(context);
    return true;
}

void TreeShinyHunting::exit_battle(BotBaseContext& context){
    pbf_press_button(context, BUTTON_B, 20, 100);
    pbf_wait(context, (1 * TICKS_PER_SECOND));
    pbf_press_button(context, BUTTON_B, 20, 100);
    pbf_wait(context, (1 * TICKS_PER_SECOND));
    pbf_press_button(context, BUTTON_A, 20, 100);
    pbf_wait(context, (3 * TICKS_PER_SECOND));
    context.wait_for_all_requests();
}

void TreeShinyHunting::run_iteration(SingleSwitchProgramEnvironment& env, BotBaseContext& context){

    Stats& stats = env.stats<Stats>();
    stats.attempts++;

    int16_t match = static_cast<int>(STOP_ON) + 1;

    env.console.log("Starting...");
    {
        //Tree 1
        goto_camp_from_jubilife(env, env.console, context, TravelLocations::instance().Fieldlands_Heights);
        pbf_move_left_joystick(context, 170, 255, 30, 30);
        change_mount(env.console, context, MountState::BRAVIARY_ON);
        pbf_press_button(context, BUTTON_B, (6.35 * TICKS_PER_SECOND), 20);
        pbf_press_button(context, BUTTON_PLUS, 20, 20);
        pbf_wait(context, (1 * TICKS_PER_SECOND));
        pbf_press_button(context, BUTTON_PLUS, 20, 20);
        pbf_wait(context, (0.5 * TICKS_PER_SECOND));
        pbf_press_button(context, BUTTON_PLUS, 20, 20);
        pbf_wait(context, (1 * TICKS_PER_SECOND));
        pbf_move_left_joystick(context, 255, 127, 30, 30);
        pbf_wait(context, (0.5 * TICKS_PER_SECOND));
        pbf_press_button(context, BUTTON_ZL, 20, 20);
        pbf_wait(context, (0.5 * TICKS_PER_SECOND));
        pbf_move_right_joystick(context, 127, 255, (0.10 * TICKS_PER_SECOND), 20);
        pbf_wait(context, (0.5 * TICKS_PER_SECOND));
        context.wait_for_all_requests();
        check_tree(env, context, match);

        //Tree 2
        goto_any_camp_from_overworld(env, env.console, context, TravelLocations::instance().Fieldlands_Heights);
        pbf_move_left_joystick(context, 152, 255, 30, 30);
        change_mount(env.console, context, MountState::BRAVIARY_ON);
        pbf_press_button(context, BUTTON_B, (11.8 * TICKS_PER_SECOND), 20);
        pbf_press_button(context, BUTTON_PLUS, 20, 20);
        pbf_wait(context, (1 * TICKS_PER_SECOND));
        pbf_press_button(context, BUTTON_PLUS, 20, 20);
        pbf_wait(context, (0.5 * TICKS_PER_SECOND));
        pbf_press_button(context, BUTTON_PLUS, 20, 20);
        pbf_wait(context, (1 * TICKS_PER_SECOND));
        pbf_move_left_joystick(context, 255, 127, 30, 30);
        pbf_wait(context, (0.5 * TICKS_PER_SECOND));
        pbf_press_button(context, BUTTON_ZL, 20, 20);
        pbf_wait(context, (0.5 * TICKS_PER_SECOND));
        pbf_move_right_joystick(context, 127, 255, (0.10 * TICKS_PER_SECOND), 20);
        pbf_wait(context, (0.5 * TICKS_PER_SECOND));
        context.wait_for_all_requests();
        check_tree(env, context, match);

        //End
        env.console.log("Nothing found, returning to Jubilife!");
        goto_camp_from_overworld(env, env.console, context);
        goto_professor(env.console, context, Camp::FIELDLANDS_FIELDLANDS);
        from_professor_return_to_jubilife(env, env.console, context);
    }
}

void TreeShinyHunting::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    Stats& stats = env.stats<Stats>();

    //  Connect the controller.
    pbf_press_button(context, BUTTON_LCLICK, 5, 5);

    while (true){
        env.update_stats();
        send_program_status_notification(
            env.logger(), NOTIFICATION_STATUS,
            env.program_info(),
            "",
            stats.to_str()
        );
        try{
            run_iteration(env, context);
        }catch (OperationFailedException&){
            stats.errors++;
            pbf_press_button(context, BUTTON_HOME, 20, GameSettings::instance().GAME_TO_HOME_DELAY);
            reset_game_from_home(env, env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
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
