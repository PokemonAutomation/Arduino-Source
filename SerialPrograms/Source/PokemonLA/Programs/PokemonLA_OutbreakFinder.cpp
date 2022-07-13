/*  Outbreak Finder
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonLA/Inference/Map/PokemonLA_MapDetector.h"
#include "PokemonLA/Inference/Map/PokemonLA_SelectedRegionDetector.h"
#include "PokemonLA/Inference/Map/PokemonLA_OutbreakReader.h"
#include "PokemonLA/Resources/PokemonLA_AvailablePokemon.h"
#include "PokemonLA/Resources/PokemonLA_PokemonSprites.h"
#include "PokemonLA/Inference/Objects/PokemonLA_ButtonDetector.h"
#include "PokemonLA/Inference/Objects/PokemonLA_MMOQuestionMarkDetector.h"
#include "PokemonLA/Programs/PokemonLA_RegionNavigation.h"
#include "PokemonLA_OutbreakFinder.h"

#include "CommonFramework/ImageMatch/ImageCropper.h"
#include "Common/Qt/ImageOpener.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

namespace {

Pokemon::ExtraNames load_mmo_names(){
    Pokemon::ExtraNames names;

    const std::string slugs[5] = {
        "fieldlands-mmo",
        "mirelands-mmo",
        "coastlands-mmo",
        "highlands-mmo",
        "icelands-mmo"
    };

    const std::string display_names[5] = {
        "Fieldlands MMO",
        "Mirelands MMO",
        "Coastlands MMO",
        "Highlands MMO",
        "Icelands MMO"
    };

    QString mmo_symbol_path = RESOURCE_PATH() + "PokemonLA/MMOQuestionMark-Template.png";
    QImage mmo_sprite = open_image(mmo_symbol_path);
    QPixmap mmo_pixmap = QPixmap::fromImage(ImageMatch::trim_image_alpha(mmo_sprite));


    for(size_t i = 0; i < 5; i++){
        names.name_list.emplace_back(slugs[i]);
        names.names.emplace(slugs[i], std::make_pair(display_names[i], mmo_pixmap));
        names.display_name_to_slug.emplace(display_names[i], slugs[i]);
    }

    return names;
}

const Pokemon::ExtraNames& MMO_NAMES(){
    const static Pokemon::ExtraNames mmo_names = load_mmo_names();
    return mmo_names;
}


} // anonymous namespace


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
        ALL_POKEMON_SPRITES(),
        HISUI_OUTBREAK_SLUGS(),
        &MMO_NAMES()
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
        , mmos(m_stats["MMOs"])
        , matches(m_stats["Matches"])
    {
        m_display_order.emplace_back("Checks");
        m_display_order.emplace_back("Errors", true);
        m_display_order.emplace_back("Outbreaks");
        m_display_order.emplace_back("MMOs");
        m_display_order.emplace_back("Matches", true);
    }

    std::atomic<uint64_t>& checks;
    std::atomic<uint64_t>& errors;
    std::atomic<uint64_t>& outbreaks;
    std::atomic<uint64_t>& mmos;
    std::atomic<uint64_t>& matches;
};


std::unique_ptr<StatsTracker> OutbreakFinder::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

//  Returns true if program should stop. (match found)
//  "current_region" is set to the location of the cursor.
//  If "current_region" is set to "MapRegion::NONE", it means an inference error.
bool OutbreakFinder::read_outbreaks(
    SingleSwitchProgramEnvironment& env, BotBaseContext& context,
    MapRegion& current_region,
    const std::set<std::string>& desired
){
    Stats& stats = env.current_stats<Stats>();

    MapRegion start_region = MapRegion::NONE;

    current_region = detect_selected_region(env.console, context);
    if (current_region == MapRegion::NONE){
        env.console.log("Unable to detect selected region.", COLOR_RED);
        return false;
    }

    MMOQuestionMarkDetector question_mark_detector(env.logger());
    VideoOverlaySet mmo_overlay_set(env.console);
    std::array<bool, 5> mmo_appears = question_mark_detector.detect_MMO_on_hisui_map(env.console.video().snapshot());
    add_hisui_MMO_detection_to_overlay(mmo_appears, mmo_overlay_set);

    // If the current region is a wild area, the yellow cursor may overlap with the MMO question marker, causing
    // wrong detection. So we have to check it's location again by moving the cursor to the next location
    if (current_region != MapRegion::JUBILIFE && current_region != MapRegion::RETREAT){
        // MapRegion starts with None and JUBILIFE. Skip those two, so -2.
        size_t current_wild_area_index = (int)current_region - 2;
        pbf_press_dpad(context, DPAD_RIGHT, 20, 40);
        context.wait_for_all_requests();
        auto new_mmo_read = question_mark_detector.detect_MMO_on_hisui_map(env.console.video().snapshot());
        mmo_appears[current_wild_area_index] = new_mmo_read[current_wild_area_index];
        if (new_mmo_read[current_wild_area_index]){
            add_hisui_MMO_detection_to_overlay(mmo_appears, mmo_overlay_set);
        }
        // now mmo_appears should contain correct detection of MMO question marks.
    }

    // reset current_region to start the looping of checking each wild area's outbreak pokemon name:
    current_region = MapRegion::NONE;
    size_t matches = 0;

    // First, check whether we match MMOs:
    const auto& mmo_names = MMO_NAMES();
    for (int i = 0; i < 5; i++){
        if (mmo_appears[i]){
            auto iter = desired.find(mmo_names.name_list[i]);
            if (iter != desired.end()){
                env.console.log("Found a match!", COLOR_BLUE);
                matches++;
            }
            stats.mmos++;
        }
    }
    env.update_stats();
    
    // Next, go to each region, read the outbreak names:
    while (true){
        current_region = detect_selected_region(env.console, context);
        if (current_region == MapRegion::NONE){
            env.console.log("Unable to detect selected region.", COLOR_RED);
            return false;
        }
        if (start_region == MapRegion::NONE){
            start_region = current_region;
        }else if (start_region == current_region){
            // We've visited all the regions. Exit the loop.
            break;
        }

        if (current_region != MapRegion::JUBILIFE && current_region != MapRegion::RETREAT){
            // MapRegion starts with None and JUBILIFE. Skip those two, so -2.
            const int wild_region_index = (int)current_region - 2;
            if (mmo_appears[wild_region_index]){
                env.log(std::string(MAP_REGION_NAMES[(int)current_region]) + " have MMO.", COLOR_ORANGE);
            }
            else{
                OutbreakReader reader(env.console, LANGUAGE, env.console);
                OCR::StringMatchResult result = reader.read(env.console.video().snapshot());
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
        }

        pbf_press_dpad(context, DPAD_RIGHT, 20, 40);
        context.wait_for_all_requests();
    }

    stats.matches += matches;
    stats.checks++;
    if (matches != 0){
        return true;
    }

    while (true){
        if (current_region != MapRegion::JUBILIFE && current_region != MapRegion::RETREAT){
            break;
        }
        pbf_press_dpad(context, DPAD_RIGHT, 20, 40);
        context.wait_for_all_requests();

        current_region = detect_selected_region(env.console, context);
        if (current_region == MapRegion::NONE){
            env.console.log("Unable to detect selected region.", COLOR_RED);
            return false;
        }
    }

    return false;
}
void OutbreakFinder::goto_region_and_return(SingleSwitchProgramEnvironment& env, BotBaseContext& context, MapRegion region){
    Stats& stats = env.current_stats<Stats>();

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
    Stats& stats = env.current_stats<Stats>();

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
    send_program_status_notification(env, NOTIFICATION_STATUS);

    //  Go to region and return.
    goto_region_and_return(env, context, current_region);

    return false;
}



void OutbreakFinder::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    Stats& stats = env.current_stats<Stats>();

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
        {{"Session Stats", stats.to_str()}},
        env.console.video().snapshot()
    );

    GO_HOME_WHEN_DONE.run_end_of_program(context);
}




}
}
}
