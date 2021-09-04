/*  Pokedex Recommendation Finder
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/PokemonSwSh/PokemonSettings.h"
#include "Common/PokemonSwSh/PokemonSwShGameEntry.h"
#include "Common/PokemonSwSh/PokemonSwShDateSpam.h"
#include "CommonFramework/Inference/ImageTools.h"
#include "CommonFramework/OCR/RawOCR.h"
#include "CommonFramework/OCR/Filtering.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonSwSh_DexRecFinder.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


DexRecFinder_Descriptor::DexRecFinder_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonSwSh:DexRecFinder",
        "Dex Rec Finder",
        "SwSh-Arduino/wiki/Advanced:-DexRec",
        "Search for a " + STRING_POKEDEX + " recommendation by date-spamming.",
        FeedbackType::OPTIONAL_,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



DexRecFinder::DexRecFinder(const DexRecFinder_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , GO_HOME_WHEN_DONE(false)
    , LANGUAGE(
        "<b>Game Language:</b>",
        PokemonNameReader::instance().languages(), false
    )
    , DESIRED(
        "<b>Desired " + STRING_POKEMON + ":</b><br>Stop when it finds this " + STRING_POKEMON + ". Requires the language be set.",
        "Pokemon/Pokedex/Pokedex-National.json"
    )
    , VIEW_TIME(
        "<b>View Time:</b><br>View the " + STRING_POKEDEX + " for this long before continuing.",
        "2 * TICKS_PER_SECOND"
    )
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , ENTER_POKEDEX_TIME(
        "<b>Enter " + STRING_POKEDEX + " Time:</b><br>Wait this long for the " + STRING_POKEDEX + " to open.",
        "3 * TICKS_PER_SECOND"
    )
    , BACK_OUT_TIME(
        "<b>Back Out Time:</b><br>Mash B for this long to return to the overworld.",
        "3 * TICKS_PER_SECOND"
    )
{
    PA_ADD_OPTION(START_IN_GRIP_MENU);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);

    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(DESIRED);
    PA_ADD_OPTION(VIEW_TIME);
    PA_ADD_OPTION(m_advanced_options);
    PA_ADD_OPTION(ENTER_POKEDEX_TIME);
    PA_ADD_OPTION(BACK_OUT_TIME);
}


struct DexRecFinder::Stats : public StatsTracker{
    Stats()
        : attempts(m_stats["Attempts"])
        , errors(m_stats["Read Errors"])
        , matches(m_stats["Matches"])
    {
        m_display_order.emplace_back(Stat("Attempts"));
        m_display_order.emplace_back(Stat("Read Errors"));
        m_display_order.emplace_back(Stat("Matches"));
    }

    uint64_t& attempts;
    uint64_t& errors;
    uint64_t& matches;
};
std::unique_ptr<StatsTracker> DexRecFinder::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


void DexRecFinder::read_line(
    bool& found,
    bool& bad_read,
    Logger& logger,
    const QImage& frame,
    const ImageFloatBox& box,
    const std::set<std::string>& desired
){
    QImage image = extract_box(frame, box);
    OCR::make_OCR_filter(image).apply(image);

    OCR::MatchResult result = PokemonNameReader::instance().read_substring(LANGUAGE, image);
    result.log(logger);

    if (!result.matched || result.slugs.empty()){
        bad_read = true;
        return;
    }
    for (const std::string& hit : result.slugs){
        if (desired.find(hit) != desired.end()){
            found = true;
        }
    }
}

void DexRecFinder::program(SingleSwitchProgramEnvironment& env){
    if (START_IN_GRIP_MENU){
        grip_menu_connect_go_home(env.console);
    }else{
        pbf_press_button(env.console, BUTTON_B, 5, 5);
        pbf_press_button(env.console, BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);
    }

    std::set<std::string> desired;
    desired.insert(DESIRED.slug());

    Stats& stats = env.stats<Stats>();

    while (true){
        home_to_date_time(env.console, true, true);
        neutral_date_skip(env.console);
        settings_to_enter_game(env.console, true);
        pbf_mash_button(env.console, BUTTON_B, 90);
        pbf_press_button(env.console, BUTTON_X, 20, OVERWORLD_TO_MENU_DELAY - 20);

        if (LANGUAGE){
            env.console.botbase().wait_for_all_requests();
            InferenceBoxScope box0(env.console, ImageFloatBox(0.75, 0.531 + 0 * 0.1115, 0.18, 0.059));
            InferenceBoxScope box1(env.console, ImageFloatBox(0.75, 0.531 + 1 * 0.1115, 0.18, 0.059));
            InferenceBoxScope box2(env.console, ImageFloatBox(0.75, 0.531 + 2 * 0.1115, 0.18, 0.059));
            InferenceBoxScope box3(env.console, ImageFloatBox(0.75, 0.531 + 3 * 0.1115, 0.18, 0.059));
            pbf_press_button(env.console, BUTTON_A, 10, ENTER_POKEDEX_TIME);
            env.console.botbase().wait_for_all_requests();

            QImage frame = env.console.video().snapshot();
            bool found = false;
            bool bad_read = false;
            if (!frame.isNull()){
                read_line(found, bad_read, env.console, frame, box0, desired);
                read_line(found, bad_read, env.console, frame, box1, desired);
                read_line(found, bad_read, env.console, frame, box2, desired);
                read_line(found, bad_read, env.console, frame, box3, desired);
            }else{
                bad_read = true;
            }

            stats.attempts++;
            if (found){
                env.log("Found a match!", Qt::blue);
                stats.matches++;
                break;
            }
            if (bad_read){
                env.log("Read Errors. Pausing for user to see.", Qt::red);
                stats.errors++;
                pbf_wait(env.console, VIEW_TIME);
            }
        }else{
            stats.attempts++;
            stats.errors++;
            pbf_press_button(env.console, BUTTON_A, 10, ENTER_POKEDEX_TIME);
            pbf_wait(env.console, VIEW_TIME);
        }
        env.update_stats();

        pbf_mash_button(env.console, BUTTON_B, BACK_OUT_TIME);
        pbf_press_button(env.console, BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);
    }

    env.update_stats();

    if (GO_HOME_WHEN_DONE){
        pbf_press_button(env.console, BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);
    }

    end_program_callback(env.console);
    end_program_loop(env.console);
}



}
}
}
