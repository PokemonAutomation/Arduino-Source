/*  Tree Leap Grinder
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
#include "PokemonLA/Programs/Farming/PokemonLA_LeapGrinder.h"
#include "PokemonLA/Programs/PokemonLA_TreeActions.h"
#include "PokemonLA/Resources/PokemonLA_PokemonInfo.h"


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
       {
          "Aipom", "Burmy", "Cherrim",
          "Cherubi", "Combee", "Heracross",
          "Pachirisu", "Vespiquen", "Wormadam"
       }, 0
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

void LeapGrinder::run_iteration(SingleSwitchProgramEnvironment& env, BotBaseContext& context){

    Stats& stats = env.stats<Stats>();
    stats.attempts++;

    uint8_t stop_case = static_cast<int>(STOP_ON) + 1;
    uint8_t remaining_leaps = LEAPS;

    env.console.log("Starting...");

    switch (POKEMON)
    {
        case 1:
            setup(env, env.console, context);
            break;
    }

    bool battle_found = check_tree_for_battle(env.console, context);

    context.wait_for_all_requests();

    if (battle_found){
        PokemonDetails pokemon = get_pokemon_details(env.console, context, LANGUAGE);

        env.console.log("LOOKING FOR:" + POKEMON.case_name(POKEMON));
        env.console.log("FOUND:" + pokemon.name);
        env.console.log("IS SHINY: " + std::to_string(pokemon.is_shiny));
        env.console.log("IS ALPHA: " + std::to_string(pokemon.is_alpha));

        if (pokemon.name == POKEMON.case_name(POKEMON).trimmed()){
            env.console.log("Pokemon leaped!");
            remaining_leaps--;
            stats.leaps++;
        }else{
            env.console.log("Not the expected pokemon. Continuing.");
        }

        //Match validation
        int8_t ret = (pokemon.is_alpha + pokemon.is_shiny);
        if((pokemon.is_alpha + pokemon.is_shiny) > 0){
            if(pokemon.is_alpha) ret++;
            bool is_match = (ret == stop_case || stop_case == 4);
            on_match_found(env, env.console, context, MATCH_DETECTED_OPTIONS, is_match);
        }

        exit_battle(context);
    }

    if (remaining_leaps == 0){
        throw ProgramFinishedException();
    }

    env.console.log("Reamining Leaps:" + std::to_string(remaining_leaps));

    goto_camp_from_overworld(env, env.console, context);
    goto_professor(env.console, context, Camp::FIELDLANDS_FIELDLANDS);
    from_professor_return_to_jubilife(env, env.console, context);
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
            run_iteration(env, context);
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
