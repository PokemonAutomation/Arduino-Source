/*  Leap Grinder
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "ClientSource/Connection/BotBase.h"
#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLA/PokemonLA_Settings.h"
#include "PokemonLA/Resources/PokemonLA_NameDatabase.h"
#include "PokemonLA/Inference/Sounds/PokemonLA_ShinySoundDetector.h"
#include "PokemonLA/Inference/PokemonLA_OverworldDetector.h"
#include "PokemonLA/Programs/PokemonLA_GameEntry.h"
#include "PokemonLA/Programs/Farming/PokemonLA_LeapGrinder.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{
    using namespace Pokemon;


LeapGrinder_Descriptor::LeapGrinder_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLA:Leap Grinder",
        STRING_POKEMON + " LA", "Leap Grinder",
        "ComputerControl/blob/master/Wiki/Programs/PokemonLA/LeapGrinder.md",
        "Shake trees and ores to grind tasks",
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}
class LeapGrinder_Descriptor::Stats : public StatsTracker{
public:
    Stats()
        : attempts(m_stats["Attempts"])
        , errors(m_stats["Errors"])
        , leaps(m_stats["Leaps"])
        , found(m_stats["Found"])
        , enroute_shinies(m_stats["Enroute Shinies"])
        , leap_alphas(m_stats["Leap Alphas"])
        , leap_shinies(m_stats["Leap Shinies"])
    {
        m_display_order.emplace_back("Attempts");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Leaps");
        m_display_order.emplace_back("Found");
        m_display_order.emplace_back("Enroute Shinies");
        m_display_order.emplace_back("Leap Alphas");
        m_display_order.emplace_back("Leap Shinies");
        m_aliases["Shinies"] = "Enroute Shinies";
        m_aliases["Alphas"] = "Leap Alphas";
    }

    std::atomic<uint64_t>& attempts;
    std::atomic<uint64_t>& errors;
    std::atomic<uint64_t>& leaps;
    std::atomic<uint64_t>& found;
    std::atomic<uint64_t>& enroute_shinies;
    std::atomic<uint64_t>& leap_alphas;
    std::atomic<uint64_t>& leap_shinies;
};
std::unique_ptr<StatsTracker> LeapGrinder_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


LeapGrinder::LeapGrinder()
    : LANGUAGE(
        "<b>Game Language</b>",
        Pokemon::PokemonNameReader::instance().languages(),
        LockMode::LOCK_WHILE_RUNNING,
        true
    )
    , POKEMON_DATABASE(make_name_database({
        "aipom",
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
        "bergmite",
    }))
    , POKEMON(
        "<b>Pokemon Species</b>",
        POKEMON_DATABASE,
        LockMode::LOCK_WHILE_RUNNING,
        "cherubi"
    )
    , LEAPS(
        "<b>Leaps</b> <br>How many leaps before stopping the program</br>",
        LockMode::LOCK_WHILE_RUNNING,
        1, 1, 100
    )
    , SHINY_DETECTED_ENROUTE(
        "Enroute Shiny Action",
        "This applies if a shiny is detected while enroute to destination.",
        "0 * TICKS_PER_SECOND"
    )
    , MATCH_DETECTED_OPTIONS(
        "Match Action",
        "What to do when the leaping Pokemon matches the *Stop On*.",
        "0 * TICKS_PER_SECOND"
    )
    , NOTIFICATION_STATUS("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS,
        &SHINY_DETECTED_ENROUTE.NOTIFICATIONS,
        &MATCH_DETECTED_OPTIONS.NOTIFICATIONS,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(POKEMON);
    PA_ADD_OPTION(LEAPS);
    PA_ADD_OPTION(STOP_ON);
    PA_ADD_OPTION(EXIT_METHOD);
    PA_ADD_OPTION(SHINY_DETECTED_ENROUTE);
    PA_ADD_OPTION(MATCH_DETECTED_OPTIONS);
    PA_ADD_OPTION(NOTIFICATIONS);
}


bool LeapGrinder::run_iteration(SingleSwitchProgramEnvironment& env, BotBaseContext& context){

    LeapGrinder_Descriptor::Stats& stats = env.current_stats<LeapGrinder_Descriptor::Stats>();
    stats.attempts++;

    env.console.log("Starting route and shiny detection...");

    for (size_t c = 0; true; c++){
        context.wait_for_all_requests();
        if (is_pokemon_selection(env.console, env.console.video().snapshot())){
            break;
        }
        if (c >= 5){
            throw OperationFailedException(
                ErrorReport::SEND_ERROR_REPORT, env.console,
                "Failed to switch to Pokemon selection after 5 attempts."
            );
        }
        env.console.log("Not on Pokemon selection. Attempting to switch to it...", COLOR_ORANGE);
        pbf_press_button(context, BUTTON_X, 20, 230);
    }

    float shiny_coefficient = 1.0;
    ShinySoundDetector shiny_detector(env.console, [&](float error_coefficient) -> bool{
        //  Warning: This callback will be run from a different thread than this function.
        stats.enroute_shinies++;
        shiny_coefficient = error_coefficient;
        return on_shiny_callback(env, env.console, SHINY_DETECTED_ENROUTE, error_coefficient);
    });

    int ret = run_until(
        env.console, context,
        [&](BotBaseContext& context){
            route(env, env.console, context, (LeapPokemon)POKEMON.index());
        },
        {{shiny_detector}}
    );
    shiny_detector.throw_if_no_sound();
    if (ret == 0){
        on_shiny_sound(env, env.console, context, SHINY_DETECTED_ENROUTE, shiny_coefficient);
    }

    env.console.log("End of route and shiny detection...");

    bool battle_found = check_tree_or_ore_for_battle(env.console, context);

    context.wait_for_all_requests();

    if (battle_found){
        env.console.log("Pokemon leaped!");
        stats.leaps++;

        PokemonDetails pokemon = get_pokemon_details(env.console, context, LANGUAGE);
        pbf_press_button(context, BUTTON_B, 20, 225);
        context.wait_for_all_requests();

        env.console.log("Looking for: " + POKEMON.slug());
        env.console.log("Found: " + set_to_str(pokemon.name_candidates));
        env.console.log("Gender: " + std::string(get_gender_str(pokemon.gender)));
        env.console.log("Alpha: " + std::string(pokemon.is_alpha ? "Yes" : "No"));
        env.console.log("Shiny: " + std::string(pokemon.is_shiny ? "Yes" : "No"));

        if (pokemon.name_candidates.find(POKEMON.slug()) != pokemon.name_candidates.end()){
            env.console.log("Expected Pokemon leaped!");
            stats.found++;
        }else{
            env.console.log("Not the expected pokemon.");
        }

        //  Match validation

        if (pokemon.is_alpha && pokemon.is_shiny){
            env.console.log("Found Shiny Alpha!");
            stats.leap_shinies++;
            stats.leap_alphas++;
        }else if (pokemon.is_alpha){
            env.console.log("Found Alpha!");
            stats.leap_alphas++;
        }else if (pokemon.is_shiny){
            env.console.log("Found Shiny!");
            stats.leap_shinies++;
        }else{
            env.console.log("Normie in the tree -_-");
        }
        env.update_stats();

        bool is_match = false;
        switch (STOP_ON){
        case StopOn::Shiny:
            is_match = pokemon.is_shiny;
            break;
        case StopOn::Alpha:
            is_match = pokemon.is_alpha;
            break;
        case StopOn::ShinyOrAlpha:
            is_match = pokemon.is_alpha || pokemon.is_shiny;
            break;
        case StopOn::ShinyAndAlpha:
            is_match = pokemon.is_alpha && pokemon.is_shiny;
            break;
        }

        if (pokemon.is_alpha || pokemon.is_shiny){
            on_match_found(env, env.console, context, MATCH_DETECTED_OPTIONS, is_match);
        }

        exit_battle(env.console, context, EXIT_METHOD);
    }

    env.console.log("Remaining Leaps:" + std::to_string(LEAPS - stats.leaps));

    return_to_jubilife(env, env.console, context, (LeapPokemon)POKEMON.index());

    if (stats.leaps == LEAPS){
        return true;
    }

    return false;
}

void LeapGrinder::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    LeapGrinder_Descriptor::Stats& stats = env.current_stats<LeapGrinder_Descriptor::Stats>();

    //  Connect the controller.
    pbf_press_button(context, BUTTON_LCLICK, 5, 5);

    while (true){
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS);
        try{
            if(run_iteration(env, context)){
                break;
            }
        }catch (OperationFailedException& e){
            stats.errors++;
            e.send_notification(env, NOTIFICATION_ERROR_RECOVERABLE);

            pbf_press_button(context, BUTTON_HOME, 20, GameSettings::instance().GAME_TO_HOME_DELAY);
            reset_game_from_home(env, env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
            // Switch from items to pokemons
            pbf_press_button(context, BUTTON_X, 20, 30);
        }
    }

    env.update_stats();
    pbf_press_button(context, BUTTON_HOME, 20, GameSettings::instance().GAME_TO_HOME_DELAY);
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}



}
}
}
