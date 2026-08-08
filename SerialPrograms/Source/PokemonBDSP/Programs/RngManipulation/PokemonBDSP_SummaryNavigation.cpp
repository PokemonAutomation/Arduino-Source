/*  BDSP Summary Navigation
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <chrono>
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_NatureChecker.h"
#include "PokemonBDSP/Inference/PokemonBDSP_MenuDetector.h"
#include "PokemonBDSP/Inference/PokemonBDSP_SelectionArrow.h"
#include "PokemonBDSP/Inference/Rng/PokemonBDSP_SummaryReader.h"
#include "PokemonBDSP_SummaryNavigation.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{

using namespace std::chrono_literals;


const size_t MENU_ATTEMPTS = 3;


bool open_menu(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    env.log("Mashing B through dialogue, trying the menu as we go.");
    MenuWatcher menu(COLOR_RED, true);
    int ret = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context){
            for (size_t c = 0; c < 40; c++){
                pbf_press_button(context, BUTTON_B, 100ms, 700ms);
                pbf_press_button(context, BUTTON_B, 100ms, 700ms);
                pbf_press_button(context, BUTTON_X, 100ms, 1200ms);
            }
        },
        {{menu}}
    );
    if (ret < 0){
        env.log("Never got back to the overworld menu.", COLOR_RED);
        return false;
    }
    env.log("Menu is open.");
    return true;
}


bool open_starter_summary(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    for (size_t attempt = 0; attempt < MENU_ATTEMPTS; attempt++){
        pbf_wait(context, 1200ms);
        context.wait_for_all_requests();

        env.log("Moving one entry left, then opening it.");
        pbf_press_dpad(context, DPAD_LEFT, 160ms, 840ms);
        pbf_press_button(context, BUTTON_A, 160ms, 1500ms);

        pbf_press_button(context, BUTTON_A, 100ms, 1000ms);
        context.wait_for_all_requests();

        SelectionArrowFinder arrow(env.console, {0.360, 0.110, 0.200, 0.290}, COLOR_RED);
        if (wait_until(env.console, context, 10s, {{arrow}}) >= 0){
            break;
        }
        if (attempt + 1 >= MENU_ATTEMPTS){
            env.log("Never reached the party from the menu.", COLOR_RED);
            return false;
        }
        env.log("That was not the party. Backing out and trying again.", COLOR_ORANGE);
        bool back_at_menu = false;
        for (size_t c = 0; c < 5 && !back_at_menu; c++){
            pbf_press_button(context, BUTTON_B, 160ms, 1000ms);
            context.wait_for_all_requests();
            MenuWatcher menu(COLOR_RED, true);
            back_at_menu = wait_until(env.console, context, 2s, {{menu}}) >= 0;
        }
        if (!back_at_menu){
            env.log("Backing out never returned to the menu.", COLOR_RED);
            return false;
        }
    }

    env.log("Opening the summary.");
    pbf_press_button(context, BUTTON_A, 100ms, 2000ms);
    context.wait_for_all_requests();

    env.log("Moving to the Trainer Memo page.");
    pbf_press_dpad(context, DPAD_RIGHT, 100ms, 1500ms);
    context.wait_for_all_requests();
    return true;
}


bool navigate_to_summary(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    return open_menu(env, context)
        && open_starter_summary(env, context);
}


bool read_observed_pokemon(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    Language language,
    const Pokemon::BaseStats& base_stats,
    uint8_t level,
    bool shiny,
    bool shiny_known,
    BdspObservedStarter& observed
){
    if (!navigate_to_summary(env, context)){
        env.log("Could not reach the summary, so this attempt says nothing about timing.",
            COLOR_ORANGE);
        return false;
    }

    SummaryReader reader;

    //  2nd page: nature and gender
    {
        VideoSnapshot memo = env.console.video().snapshot();
        observed.nature = reader.read_nature(env.logger(), language, memo);
        Pokemon::BdspGender gender = reader.read_gender(env.logger(), memo);
        observed.gender = gender;
        observed.gender_known = gender != Pokemon::BdspGender::Genderless;
    }
    //  3rd page: stats
    {
        pbf_press_dpad(context, DPAD_RIGHT, 100ms, 1500ms);
        context.wait_for_all_requests();
        VideoSnapshot skills = env.console.video().snapshot();
        observed.stats = reader.read_stats(env.logger(), skills);
    }
    observed.base_stats = base_stats;
    observed.level = level;
    observed.shiny = shiny;
    observed.shiny_known = shiny_known;

    auto stat = [](int16_t value){
        return value < 0 ? std::string("?") : std::to_string(value);
    };
    bool nature_read = observed.nature != Pokemon::NatureCheckerValue::UnableToDetect;
    env.log(
        "Read from the summary: nature "
        + (nature_read
            ? Pokemon::NATURE_CHECKER_VALUE_STRINGS().get_string(observed.nature)
            : std::string("UNREAD"))
        + ", gender "
        + (observed.gender_known
            ? std::string(Pokemon::bdsp_gender_name(observed.gender))
            : std::string("UNREAD"))
        + ", stats " + stat(observed.stats.hp) + "/" + stat(observed.stats.attack)
        + "/" + stat(observed.stats.defense) + "/" + stat(observed.stats.spatk)
        + "/" + stat(observed.stats.spdef) + "/" + stat(observed.stats.speed) + ".",
        nature_read && observed.gender_known ? COLOR_BLUE : COLOR_ORANGE
    );
    return true;
}




}
}
}
