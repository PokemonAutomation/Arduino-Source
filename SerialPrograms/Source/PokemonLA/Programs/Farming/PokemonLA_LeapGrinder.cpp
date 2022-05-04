/*  Leap Grinder
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */


#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "PokemonLA/PokemonLA_Settings.h"
#include "PokemonLA/Inference/Sounds/PokemonLA_ShinySoundDetector.h"
#include "PokemonLA/Programs/PokemonLA_GameEntry.h"
#include "PokemonLA/Programs/Farming/PokemonLA_LeapGrinder.h"


#include "PokemonLA/Resources/PokemonLA_AvailablePokemon.h"
#include "PokemonLA/Resources/PokemonLA_PokemonIcons.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


LeapGrinder_Descriptor::LeapGrinder_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonLA:Leap Grinder",
        STRING_POKEMON + " LA", "Leap Grinder",
        "ComputerControl/blob/master/Wiki/Programs/PokemonLA/LeapGrinder.md",
        "Shake trees to grind dex tasks",
        FeedbackType::REQUIRED, false,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}


LeapGrinder::LeapGrinder(const LeapGrinder_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , LANGUAGE("<b>Game Language</b>", Pokemon::PokemonNameReader::instance().languages(), true)
    , POKEMON(
        "<b>Pokemon Species</b>",
        {"aipom",
          "burmy",
          "cherrim",
          "cherubi",
          "combee",
          "heracross",
          "pachirisu",
          "vespiquen",
          "wormadam",
          "geodude",
          "graveler",
          "bonsly",
          "bronzor",
          "nosepass",
          "bergmite"}
        )
    , LEAPS(
      "<b>Leaps</b> <br>How many leaps before stopping the program</br>",
      1, 1, 100
      )
    , STOP_ON(
        "<b>Stop On:</b>",
        {
        "Shiny", "Alpha",
        "Shiny & Alpha",
        "Stop on any non regular"
        }, 0
    )
    , MATCH_DETECTED_OPTIONS(
      "Match Action",
      "What to do when the leaping Pokemon matches the *Stop On*",
      "0 * TICKS_PER_SECOND"
    )
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
    PA_ADD_OPTION(POKEMON);
    PA_ADD_OPTION(LEAPS);
    PA_ADD_OPTION(STOP_ON);
    PA_ADD_OPTION(MATCH_DETECTED_OPTIONS);
    PA_ADD_OPTION(SHINY_DETECTED_ENROUTE);
    PA_ADD_OPTION(NOTIFICATIONS);
}


class LeapGrinder::Stats : public StatsTracker, public ShinyStatIncrementer{
public:
    Stats()
        : attempts(m_stats["Attempts"])
        , errors(m_stats["Errors"])
        , leaps(m_stats["Leaps"])
        , shinies(m_stats["Shinies"])
        , alphas(m_stats["Alphas"])
    {
        m_display_order.emplace_back("Attempts");
        m_display_order.emplace_back("Errors", true);
        m_display_order.emplace_back("Leaps", true);
        m_display_order.emplace_back("Shinies", true);
        m_display_order.emplace_back("Alphas", true);
    }

    virtual void add_shiny() override{
        shinies++;
    }

    std::atomic<uint64_t>& attempts;
    std::atomic<uint64_t>& errors;
    std::atomic<uint64_t>& leaps;
    std::atomic<uint64_t>& shinies;
    std::atomic<uint64_t>& alphas;
};

std::unique_ptr<StatsTracker> LeapGrinder::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

bool LeapGrinder::run_iteration(SingleSwitchProgramEnvironment& env, BotBaseContext& context){

    Stats& stats = env.stats<Stats>();
    stats.attempts++;

    size_t stop_case = STOP_ON + 1;

    env.console.log("Starting route and shiny detection...");

    float shiny_coefficient = 1.0;
    ShinySoundDetector shiny_detector(env.console.logger(), env.console, [&](float error_coefficient) -> bool{
        //  Warning: This callback will be run from a different thread than this function.
        stats.shinies++;
        shiny_coefficient = error_coefficient;
        return on_shiny_callback(env, env.console, SHINY_DETECTED_ENROUTE, error_coefficient);
    });

    int ret = run_until(
        env.console, context,
        [&](BotBaseContext& context){
            route(env, env.console, context, POKEMON);
        },
        {{shiny_detector}}
    );
    shiny_detector.throw_if_no_sound();
    if (ret == 0){
        on_shiny_sound(env, env.console, context, SHINY_DETECTED_ENROUTE, shiny_coefficient);
    }

    env.console.log("End of route and shiny detection...");

    bool battle_found = check_tree_for_battle(env.console, context);

    context.wait_for_all_requests();

    if (battle_found){
        env.console.log("Pokemon leaped!");

        PokemonDetails pokemon = get_pokemon_details(env.console, context, LANGUAGE);

        env.console.log("Looking for: " + POKEMON.slug());
        env.console.log("Found: " + pokemon.name);
        env.console.log("Gender: " + pokemon.gender);
        env.console.log("Alpha: " + std::to_string(pokemon.is_alpha));
        env.console.log("Shiny: " + std::to_string(pokemon.is_shiny));

        //if (pokemon.name.compare(POKEMON.case_name(POKEMON).trimmed(),Qt::CaseInsensitive) == 0){
        if (pokemon.name == QString::fromStdString(POKEMON.slug())){
            env.console.log("Expected Pokemon leaped!");
            stats.leaps++;
        }else{
            env.console.log("Not the expected pokemon.");
        }

        //Match validation
        size_t match_ret = (pokemon.is_alpha + pokemon.is_shiny);
        if((pokemon.is_alpha + pokemon.is_shiny) > 0){
            if(pokemon.is_alpha) match_ret++;
            bool is_match = (match_ret == stop_case || stop_case == 4);
            on_match_found(env, env.console, context, MATCH_DETECTED_OPTIONS, is_match);
        }

        exit_battle(context);
    }

    env.console.log("Remaining Leaps:" + std::to_string(LEAPS - stats.leaps));

    return_to_jubilife(env, env.console, context, POKEMON);

    if (stats.leaps == LEAPS){
        return true;
    }

    return false;
}

bool LeapGrinder::quick_check(SingleSwitchProgramEnvironment& env, BotBaseContext& context){

    PokemonDetails pokemon = get_pokemon_details(env.console, context, LANGUAGE);

    env.console.log("Looking for: " + POKEMON.slug());
    env.console.log("Found: " + pokemon.name);
    env.console.log("Gender: " + pokemon.gender);
    env.console.log("Alpha: " + std::to_string(pokemon.is_alpha));
    env.console.log("Shiny: " + std::to_string(pokemon.is_shiny));
    if (pokemon.name == QString::fromStdString(POKEMON.slug())){
        env.console.log("Expected Pokemon leaped!");

    }else{
        env.console.log("Not the expected pokemon.");
    }

    return true;
}


void LeapGrinder::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
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
            if(run_iteration(env, context))
            //if(quick_check(env, context))
                break;
        }catch (OperationFailedException&){
            stats.errors++;
            pbf_press_button(context, BUTTON_HOME, 20, GameSettings::instance().GAME_TO_HOME_DELAY);
            reset_game_from_home(env, env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
        }
    }

    env.update_stats();
    pbf_press_button(context, BUTTON_HOME, 20, GameSettings::instance().GAME_TO_HOME_DELAY);
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
