/*  Burmy Hunter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "PokemonLA/PokemonLA_Settings.h"
#include "PokemonLA/Inference/PokemonLA_StatusInfoScreenDetector.h"
#include "PokemonLA/Inference/Sounds/PokemonLA_ShinySoundDetector.h"
#include "PokemonLA/Programs/PokemonLA_MountChange.h"
#include "PokemonLA/Programs/PokemonLA_GameEntry.h"
#include "PokemonLA/Programs/ShinyHunting/PokemonLA_BurmyFinder.h"
#include "PokemonLA/Programs/PokemonLA_TreeActions.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


BurmyFinder_Descriptor::BurmyFinder_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonLA:Burmy Hunter",
        STRING_POKEMON + " LA", "Burmy Hunter",
        "ComputerControl/blob/master/Wiki/Programs/PokemonLA/BurmyHunter.md",
        "Check nearby trees for a possible Shiny, Alpha or Alpha Shiny Burmy",
        FeedbackType::REQUIRED, false,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}


BurmyFinder::BurmyFinder(const BurmyFinder_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , LANGUAGE("<b>Game Language</b>", Pokemon::PokemonNameReader::instance().languages(), true)
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
    , MATCH_DETECTED_OPTIONS(
      "Match Action",
      "What to do when the pokemon matches the expectations",
      "0 * TICKS_PER_SECOND")

    , SHINY_DETECTED_ENROUTE(
        "Enroute Shiny Action",
        "This applies if a shiny is detected while enroute to destination",
        "0 * TICKS_PER_SECOND"
    )
    , NOTIFICATION_STATUS("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS,
        &MATCH_DETECTED_OPTIONS.NOTIFICATIONS,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(STOP_ON);
    PA_ADD_OPTION(MATCH_DETECTED_OPTIONS);
    PA_ADD_OPTION(SHINY_DETECTED_ENROUTE);
    PA_ADD_OPTION(NOTIFICATIONS);
}


class BurmyFinder::Stats : public StatsTracker, public ShinyStatIncrementer{
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

std::unique_ptr<StatsTracker> BurmyFinder::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


void BurmyFinder::check_tree(SingleSwitchProgramEnvironment& env, BotBaseContext& context, int16_t expected){
    Stats& stats = env.stats<Stats>();

    bool battle_found = check_tree_for_battle(env.console, context);

    context.wait_for_all_requests();

    if (battle_found){
        PokemonDetails pokemon = get_pokemon_details(env.console, context, LANGUAGE);

        //Match validation
        int8_t ret = (pokemon.is_alpha + pokemon.is_shiny);
        if(ret > 0){
            if(pokemon.is_alpha) ret++;
            switch (ret) {
                case 1:
                    env.console.log("Found Shiny!");
                    stats.shinies++;
                    break;

                case 2:
                    env.console.log("Found Alpha!");
                    stats.alphas++;
                    break;

                case 3:
                     env.console.log("Found Shiny Alpha!");
                     stats.shinies++;
                     stats.alphas++;
                    break;
            }
            bool is_match = (ret == expected || expected == 4);
            on_match_found(env, env.console, context, MATCH_DETECTED_OPTIONS, is_match);
        }
        else{
            env.console.log("Normie in the tree -_-");
            stats.found++;
        }

        exit_battle(context);
    }
}

void BurmyFinder::run_iteration(SingleSwitchProgramEnvironment& env, BotBaseContext& context){

    Stats& stats = env.stats<Stats>();
    stats.attempts++;

    int16_t stop_case = STOP_ON + 1;

    env.console.log("Starting route and shiny detection...");

    float shiny_coefficient = 1.0;
    ShinySoundDetector shiny_detector(env.console, [&](float error_coefficient) -> bool{
        //  Warning: This callback will be run from a different thread than this function.
        stats.shinies++;
        shiny_coefficient = error_coefficient;
        return on_shiny_callback(env, env.console, SHINY_DETECTED_ENROUTE, error_coefficient);
    });

    int ret = run_until(
        env.console, context,
        [&](BotBaseContext& context){

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
            check_tree(env, context, stop_case);

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
            check_tree(env, context, stop_case);

        },
        {{shiny_detector}}
    );
    if (ret == 0){
        on_shiny_sound(env, env.console, context, SHINY_DETECTED_ENROUTE, shiny_coefficient);
    }

    //End
    env.console.log("Nothing found, returning to Jubilife!");
    goto_camp_from_overworld(env, env.console, context);
    goto_professor(env.console, context, Camp::FIELDLANDS_FIELDLANDS);
    from_professor_return_to_jubilife(env, env.console, context);

}

void BurmyFinder::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
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
