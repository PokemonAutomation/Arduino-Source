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
#include "PokemonLA/Inference/PokemonLA_BattleMenuDetector.h"
#include "PokemonLA/Inference/PokemonLA_BattlePokemonSwitchDetector.h"
#include "PokemonLA/Inference/PokemonLA_StatusInfoScreenDetector.h"
#include "PokemonLA/Inference/Objects/PokemonLA_ButtonDetector.h"
#include "PokemonLA/Inference/PokemonLA_MapDetector.h"
#include "PokemonLA/Inference/PokemonLA_ShinySoundDetector.h"
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
        "Constantly reset to find a Shiny Unown or any Shiny in the path.",
        FeedbackType::REQUIRED, false,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}


TreeShinyHunting::TreeShinyHunting(const TreeShinyHunting_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , SHINY_DETECTED("0 * TICKS_PER_SECOND")
    , NOTIFICATION_STATUS("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS,
        &SHINY_DETECTED.NOTIFICATIONS,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(SHINY_DETECTED);
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

    virtual void add_alpha() {
        alphas++;
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

int16_t check_for_battle(SingleSwitchProgramEnvironment& env, BotBaseContext& context){

    pbf_press_button(context, BUTTON_ZR, (0.5 * TICKS_PER_SECOND), 20); //throw pokemon
    pbf_wait(context, (4 * TICKS_PER_SECOND));
    context.wait_for_all_requests();

    MountDetector mount_detector;
    MountState mount = mount_detector.detect(env.console.video().snapshot());

    if (mount != MountState::NOTHING){
       env.console.log("Battle not found. Tree might be empty.");
       dump_image(env.console, env.program_info(), "Nothing On Tree", env.console.video().snapshot());
    }
    else{
       env.console.log("Found battle");
       dump_image(env.console, env.program_info(), "Battle Found", env.console.video().snapshot());

       BattleMenuDetector battle_menu_detector(env.console, env.console, true);
       wait_until(
           env, env.console, context, std::chrono::seconds(5),
           {
               &battle_menu_detector,
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

       dump_image(env.console, env.program_info(), "Details", infoScreen);
       detector.process_frame(infoScreen, std::chrono::system_clock::now());

       int16_t ret = detector.detected();

       env.console.log("DETECTOR RETURNED: " + std::to_string(ret));
       context.wait_for_all_requests();
       return ret;
    }

    return -1;
}

void TreeShinyHunting::run_iteration(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    Stats& stats = env.stats<Stats>();
    stats.attempts++;

    env.console.log("Beginning Shiny Detection...");
    {
        goto_camp_from_jubilife(env, env.console, context, TravelLocations::instance().Fieldlands_Heights);

        //First Tree
        pbf_move_left_joystick(context, 200, 255, 30, 30);
        change_mount(env.console, context, MountState::BRAVIARY_ON);
        pbf_press_button(context, BUTTON_B, (3 * TICKS_PER_SECOND), 20);//turn right
        change_mount(env.console, context, MountState::BRAVIARY_OFF);

        pbf_move_right_joystick(context, 0, 127, (0.5 * TICKS_PER_SECOND), 30);
        pbf_move_left_joystick(context, 0, 127, 30, 30);
        pbf_press_button(context, BUTTON_ZL, 30, 30);

        context.wait_for_all_requests();

        int16_t ret = check_for_battle(env, context);

        switch (ret) {
            case 0:
                env.console.log("Normie in the tree -_-");
                pbf_press_button(context, BUTTON_B, 20, 100);
                pbf_wait(context, (1 * TICKS_PER_SECOND));
                pbf_press_button(context, BUTTON_B, 20, 100);
                pbf_wait(context, (1 * TICKS_PER_SECOND));
                pbf_press_button(context, BUTTON_A, 20, 100);
                pbf_wait(context, (3 * TICKS_PER_SECOND));
                stats.found++;
                context.wait_for_all_requests();
                break;
            case 1:
                env.console.log("Found SHINY");
                stats.shinies++;
                throw OperationFailedException(env.console, "SHINY");

            case 2:
                env.console.log("FOUND ALPHA");
                stats.alphas++;
                throw OperationFailedException(env.console, "ALPHA");

            case 3:
                env.console.log("FOUND SHINY ALPHA");
                stats.shinies++;
                stats.alphas++;
                throw OperationFailedException(env.console, "SHINY ALPHA");
        }

        //Second Tree
        goto_any_camp_from_overworld(env, env.console, context, TravelLocations::instance().Fieldlands_Heights);

        pbf_move_left_joystick(context, 137, 255, 30, 30);//turn back
        change_mount(env.console, context, MountState::BRAVIARY_ON);
        pbf_press_button(context, BUTTON_B, (6.25 * TICKS_PER_SECOND), 20); //Ride forward

        pbf_press_button(context, BUTTON_PLUS, 20, 20); //Braviary off
        pbf_wait(context, (1 * TICKS_PER_SECOND));
        pbf_press_button(context, BUTTON_PLUS, 20, 20); //Braviary on
        pbf_wait(context, (0.5 * TICKS_PER_SECOND));
        pbf_press_button(context, BUTTON_PLUS, 20, 20); //Braviary off
        pbf_wait(context, (1 * TICKS_PER_SECOND));

        pbf_move_left_joystick(context, 0, 127, 30, 30);//look to tree
        pbf_wait(context, (0.5 * TICKS_PER_SECOND));
        pbf_press_button(context, BUTTON_ZL, 30, 30);   //focus
        pbf_wait(context, (0.5 * TICKS_PER_SECOND));
        pbf_move_right_joystick(context, 127, 255, (0.15 * TICKS_PER_SECOND), 20);
        pbf_wait(context, (0.5 * TICKS_PER_SECOND));

        context.wait_for_all_requests();

        ret = check_for_battle(env, context);

        switch (ret) {
            case 0:
                env.console.log("Normie in the tree -_-");
                pbf_press_button(context, BUTTON_B, 20, 100);
                pbf_wait(context, (1 * TICKS_PER_SECOND));
                pbf_press_button(context, BUTTON_B, 20, 100);
                pbf_wait(context, (1 * TICKS_PER_SECOND));
                pbf_press_button(context, BUTTON_A, 20, 100);
                pbf_wait(context, (3 * TICKS_PER_SECOND));
                stats.found++;
                context.wait_for_all_requests();
                break;
            case 1:
                env.console.log("Found SHINY");
                stats.shinies++;
                throw OperationFailedException(env.console, "SHINY");

            case 2:
                env.console.log("FOUND ALPHA");
                stats.alphas++;
                throw OperationFailedException(env.console, "ALPHA");

            case 3:
                env.console.log("FOUND SHINY ALPHA");
                stats.shinies++;
                stats.alphas++;
                throw OperationFailedException(env.console, "SHINY ALPHA");
        }



        //End
        env.console.log("Noothing found, returning to Jubilife!");
        goto_camp_from_overworld(env, env.console, context, SHINY_DETECTED, stats);
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
            //break;
            pbf_press_button(context, BUTTON_HOME, 20, 20);
            break;
            //stats.errors++;
            //pbf_press_button(context, BUTTON_HOME, 20, GameSettings::instance().GAME_TO_HOME_DELAY);
            //reset_game_from_home(env, env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
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
