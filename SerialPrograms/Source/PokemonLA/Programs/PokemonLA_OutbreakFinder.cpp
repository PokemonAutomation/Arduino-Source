/*  Outbreak Finder
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonLA/Resources/PokemonLA_AvailablePokemon.h"
#include "PokemonLA/Resources/PokemonLA_PokemonIcons.h"
#include "PokemonLA/Inference/Objects/PokemonLA_ButtonDetector.h"
#include "PokemonLA/Inference/PokemonLA_MapDetector.h"
#include "PokemonLA/Inference/PokemonLA_SelectedRegionDetector.h"
#include "PokemonLA/Inference/PokemonLA_OutbreakReader.h"
#include "PokemonLA/Programs/PokemonLA_RegionNavigation.h"
#include "PokemonLA_OutbreakFinder.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


OutbreakFinder_Descriptor::OutbreakFinder_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonLA:OutbreakFinder",
        STRING_POKEMON + " LA", "Outbreak Finder",
        "ComputerControl/blob/master/Wiki/Programs/PokemonLA/OutbreakFinder.md",
        "Search for an outbreak for a specific " + STRING_POKEMON,
        FeedbackType::REQUIRED, false,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}


OutbreakFinder::OutbreakFinder(const OutbreakFinder_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , GO_HOME_WHEN_DONE(false)
    , LANGUAGE("<b>Game Language:</b>", Pokemon::PokemonNameReader::instance().languages(), true)
    , DESIRED_SLUGS(
        "<b>Desired " + STRING_POKEMON + ":</b><br>Stop when anything on this list is found.",
        ALL_POKEMON_ICONS(),
        HISUI_DEX_SLUGS()
    )
    , NOTIFICATION_STATUS("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATION_MATCHED(
        "Match Found",
        true, true, ImageAttachmentMode::JPG,
        {"Notifs"}
    )
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS,
        &NOTIFICATION_MATCHED,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(DESIRED_SLUGS);
    PA_ADD_OPTION(NOTIFICATIONS);
}


class OutbreakFinder::Stats : public StatsTracker{
public:
    Stats()
        : checks(m_stats["Checks"])
        , errors(m_stats["Errors"])
        , outbreaks(m_stats["Outbreaks"])
        , matches(m_stats["Matches"])
    {
        m_display_order.emplace_back("Checks");
        m_display_order.emplace_back("Errors", true);
        m_display_order.emplace_back("Outbreaks");
        m_display_order.emplace_back("Matches", true);
    }

    std::atomic<uint64_t>& checks;
    std::atomic<uint64_t>& errors;
    std::atomic<uint64_t>& outbreaks;
    std::atomic<uint64_t>& matches;
};


std::unique_ptr<StatsTracker> OutbreakFinder::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


bool OutbreakFinder::read_outbreaks(
    SingleSwitchProgramEnvironment& env, BotBaseContext& context,
    MapRegion& current_region,
    const std::set<std::string>& desired
){
    //  Returns true if program should stop. (match found)
    //  "current_region" is set to the location of the cursor.
    //  If "current_region" is set to "MapRegion::NONE", it means an inference error.

    Stats& stats = env.stats<Stats>();

    MapRegion start_region = MapRegion::NONE;
    MapRegion no_outbreak = MapRegion::NONE;

    current_region = MapRegion::NONE;
    size_t matches = 0;
    while (true){
        current_region = detect_selected_region(env.console, context);
        if (current_region == MapRegion::NONE){
            env.console.log("Unable to detect selected region.", COLOR_RED);
            return false;
        }
        if (start_region == MapRegion::NONE){
            start_region = current_region;
        }else if (start_region == current_region){
            break;
        }

        if (current_region != MapRegion::JUBILIFE && current_region != MapRegion::RETREAT){
            OutbreakReader reader(env.console, LANGUAGE, env.console);
            OCR::StringMatchResult result = reader.read(env.console.video().snapshot());
            if (no_outbreak == MapRegion::NONE && result.results.empty()){
                no_outbreak = current_region;
            }
            if (!result.results.empty()){
                stats.outbreaks++;
            }
            for (const auto& item : result.results){
                auto iter = desired.find(item.second.token);
                if (iter != desired.end()){
                    env.console.log("Found a match!", COLOR_BLUE);
                    matches++;
                }
            }
        }

        pbf_press_dpad(context, DPAD_RIGHT, 20, 40);
        context.wait_for_all_requests();
    }

    stats.matches += matches;
    stats.checks++;
    if (matches != 0){
        return true;
    }

    //  Scroll to next region without an outbreak.
    if (no_outbreak != MapRegion::NONE){
        while (true){
            current_region = detect_selected_region(env.console, context);
            if (current_region == MapRegion::NONE){
                env.console.log("Unable to detect selected region.", COLOR_RED);
                return false;
            }
            if (current_region == no_outbreak){
                break;
            }
            pbf_press_dpad(context, DPAD_RIGHT, 20, 40);
            context.wait_for_all_requests();
        }
    }

    return false;
}
void OutbreakFinder::goto_region_and_return(SingleSwitchProgramEnvironment& env, BotBaseContext& context, MapRegion region){
    Stats& stats = env.stats<Stats>();

    mash_A_to_change_region(env, env.console, context);

    Camp camp = Camp::FIELDLANDS_FIELDLANDS;
    switch (region){
    case MapRegion::FIELDLANDS:
        camp = Camp::FIELDLANDS_FIELDLANDS;
        break;
    case MapRegion::MIRELANDS:
        camp = Camp::MIRELANDS_MIRELANDS;
        break;
    case MapRegion::COASTLANDS:
        camp = Camp::COASTLANDS_BEACHSIDE;
        break;
    case MapRegion::HIGHLANDS:
        camp = Camp::HIGHLANDS_HIGHLANDS;
        break;
    case MapRegion::ICELANDS:
        camp = Camp::ICELANDS_SNOWFIELDS;
        break;
    default:
        throw InternalProgramError(&env.console.logger(), PA_CURRENT_FUNCTION, "Invalid region.");
    }
    goto_professor(env.console, context, camp);


    while (true){
        context.wait_for_all_requests();
        ButtonDetector button_detector(
            env.console, env.console,
            ButtonType::ButtonA, ImageFloatBox(0.50, 0.50, 0.30, 0.30),
            std::chrono::milliseconds(200), true
        );
        int ret = run_until(
            env.console, context,
            [](BotBaseContext& context){
                for (size_t c = 0; c < 10; c++){
                    pbf_press_button(context, BUTTON_A, 20, 125);
                }
            },
            {{button_detector}}
        );
        if (ret >= 0){
            context.wait_for(std::chrono::milliseconds(500));
            pbf_press_dpad(context, DPAD_DOWN, 20, 105);
            break;
        }
        env.console.log("Did not detect option to return to Jubilife.", COLOR_RED);
        pbf_mash_button(context, BUTTON_B, 5 * TICKS_PER_SECOND);
        stats.errors++;
    }

    mash_A_to_change_region(env, env.console, context);
}


bool OutbreakFinder::run_iteration(
    SingleSwitchProgramEnvironment& env, BotBaseContext& context,
    const std::set<std::string>& desired
){
    Stats& stats = env.stats<Stats>();

    //  Enter map.
    pbf_move_left_joystick(context, 128, 255, 200, 0);

    MapDetector detector;
    int ret = run_until(
        env.console, context,
        [](BotBaseContext& context){
            for (size_t c = 0; c < 10; c++){
                pbf_press_button(context, BUTTON_A, 20, 105);
            }
        },
        {{detector}}
    );
    if (ret < 0){
        stats.errors++;
        throw OperationFailedException(env.console, "Map not detected after 10 x A presses.");
    }
    env.console.log("Found map!");
    context.wait_for(std::chrono::milliseconds(500));


    MapRegion current_region;
    if (read_outbreaks(env, context, current_region, desired)){
        return true;
    }
    if (current_region == MapRegion::NONE){
        stats.errors++;
        throw OperationFailedException(env.console, "Unable to read map.");
    }

    env.update_stats();
    send_program_status_notification(
        env.console, NOTIFICATION_STATUS,
        env.program_info(),
        "",
        stats.to_str()
    );

    //  Go to region and return.
    goto_region_and_return(env, context, current_region);

    return false;
}



void OutbreakFinder::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    Stats& stats = env.stats<Stats>();

    std::set<std::string> desired;
    for (size_t c = 0; c < DESIRED_SLUGS.size(); c++){
        desired.insert(DESIRED_SLUGS[c]);
    }


    while (true){
        if (run_iteration(env, context, desired)){
            break;
        }
    }

    env.update_stats();

    send_program_notification(
        env.console, NOTIFICATION_MATCHED,
        COLOR_GREEN,
        env.program_info(),
        "Found Outbreak",
        {{"Session Stats", QString::fromStdString(stats.to_str())}},
        env.console.video().snapshot()
    );

    GO_HOME_WHEN_DONE.run_end_of_program(context);
}




}
}
}
