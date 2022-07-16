/*  Outbreak Finder
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Qt/ImageOpener.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageMatch/ImageCropper.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonLA/Inference/Map/PokemonLA_MapDetector.h"
#include "PokemonLA/Inference/Map/PokemonLA_SelectedRegionDetector.h"
#include "PokemonLA/Inference/Map/PokemonLA_OutbreakReader.h"
#include "PokemonLA/Inference/Map/PokemonLA_MapMissionTabReader.h"
#include "PokemonLA/Inference/Map/PokemonLA_MapZoomLevelReader.h"
#include "PokemonLA/Inference/Map/PokemonLA_MMOSpriteStarSymbolDetector.h"
#include "PokemonLA/Inference/Map/PokemonLA_PokemonMapSpriteReader.h"
#include "PokemonLA/Inference/Objects/PokemonLA_DialogueYellowArrowDetector.h"
#include "PokemonLA/Programs/PokemonLA_GameEntry.h"
#include "PokemonLA/Resources/PokemonLA_AvailablePokemon.h"
#include "PokemonLA/Resources/PokemonLA_PokemonSprites.h"
#include "PokemonLA/Inference/Objects/PokemonLA_ButtonDetector.h"
#include "PokemonLA/Inference/Objects/PokemonLA_MMOQuestionMarkDetector.h"
#include "PokemonLA_OutbreakFinder.h"
#include "PokemonLA/Programs/PokemonLA_RegionNavigation.h"
#include "PokemonLA/PokemonLA_Settings.h"

#include <sstream>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{
    using namespace Pokemon;



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

    std::string mmo_symbol_path = RESOURCE_PATH() + "PokemonLA/MMOQuestionMark-Template.png";
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
    , DESIRED_MO_SLUGS(
        "<b>Desired Outbreak " + STRING_POKEMON + ":</b><br>Stop when anything on this list is found.",
        ALL_POKEMON_SPRITES(),
        HISUI_OUTBREAK_SLUGS(),
        nullptr,
        &MMO_NAMES()
    )
    , DESIRED_MMO_SLUGS(
        "<b>Desired first MMO wave " + STRING_POKEMON + ":</b><br>Stop when anything on this list is found.",
        ALL_MMO_SPRITES(),
        MMO_FIRST_WAVE_SPRITE_SLUGS(),
        &MMO_FIRST_WAVE_DISPLAY_NAME_MAPPING()
    )
    , DESIRED_STAR_MMO_SLUGS(
        "<b>Desired first MMO wave " + STRING_POKEMON + " with shiny symbols:</b><br>Stop when anything on this list is found.",
        ALL_MMO_SPRITES(),
        MMO_FIRST_WAVE_SPRITE_SLUGS(),
        &MMO_FIRST_WAVE_DISPLAY_NAME_MAPPING()
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
    PA_ADD_OPTION(DESIRED_MO_SLUGS);
    PA_ADD_OPTION(DESIRED_MMO_SLUGS);
    PA_ADD_OPTION(DESIRED_STAR_MMO_SLUGS);
    PA_ADD_OPTION(NOTIFICATIONS);
}


class OutbreakFinder::Stats : public StatsTracker{
public:
    Stats()
        : checks(m_stats["Checks"])
        , errors(m_stats["Errors"])
        , outbreaks(m_stats["Outbreaks"])
        , mmos(m_stats["MMOs"])
        , mmo_pokemon(m_stats["MMO Pokemon"])
        , stars(m_stats["Stars"])
        , matches(m_stats["Matches"])
    {
        m_display_order.emplace_back("Checks");
        m_display_order.emplace_back("Errors", true);
        m_display_order.emplace_back("Outbreaks");
        m_display_order.emplace_back("MMOs");
        m_display_order.emplace_back("MMO Pokemon");
        m_display_order.emplace_back("Stars");
        m_display_order.emplace_back("Matches", true);
    }

    std::atomic<uint64_t>& checks;
    std::atomic<uint64_t>& errors;
    std::atomic<uint64_t>& outbreaks;
    std::atomic<uint64_t>& mmos;
    std::atomic<uint64_t>& mmo_pokemon;
    std::atomic<uint64_t>& stars; // MMO star symbols
    std::atomic<uint64_t>& matches;
};


std::unique_ptr<StatsTracker> OutbreakFinder::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

//  Returns true if program should stop. (match found)
//  "current_region" is set to the location of the cursor.
//  If "current_region" is set to "MapRegion::NONE", it means an inference error.
std::set<std::string> OutbreakFinder::read_outbreaks(
    SingleSwitchProgramEnvironment& env, BotBaseContext& context,
    MapRegion& current_region,
    const std::set<std::string>& desired
){
    Stats& stats = env.current_stats<Stats>();

    MapRegion start_region = MapRegion::NONE;

    current_region = detect_selected_region(env.console, context);
    if (current_region == MapRegion::NONE){
        env.console.log("Unable to detect selected region.", COLOR_RED);
        return std::set<std::string>();
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

    std::set<std::string> found;

    // First, check whether we match MMOs:
    const auto& mmo_names = MMO_NAMES();
    for (int i = 0; i < 5; i++){
        if (mmo_appears[i]){
            auto iter = desired.find(mmo_names.name_list[i]);
            if (iter != desired.end()){
                env.console.log("Found a match!", COLOR_BLUE);
                found.insert(mmo_names.name_list[i]);
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
            return std::set<std::string>();
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
                        found.insert(item.second.token);
                    }
                }
            }
        }

        pbf_press_dpad(context, DPAD_RIGHT, 20, 40);
        context.wait_for_all_requests();
    }

    stats.checks++;

    if (found.size() > 0){
        return found;
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
            return std::set<std::string>();
        }
    }

    return std::set<std::string>();
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

std::set<std::string> OutbreakFinder::read_MMOs(
    SingleSwitchProgramEnvironment& env, BotBaseContext& context,
    const std::string& mmo_name,
    const std::set<std::string>& desired_MMOs,
    const std::set<std::string>& desired_star_MMOs
){
    Stats& stats = env.current_stats<Stats>();

    MapRegion region = MapRegion::NONE;
    TravelLocation location = TravelLocations::instance().Fieldlands_Fieldlands;
    Camp camp = Camp::FIELDLANDS_FIELDLANDS;
    for(size_t i = 0; i < 5; i++){
        if (mmo_name == MMO_NAMES().name_list[i]){
            switch (i){
            case 0:
                region = MapRegion::FIELDLANDS;
                location = TravelLocations::instance().Fieldlands_Fieldlands;
                camp = Camp::FIELDLANDS_FIELDLANDS;
                break;
            case 1:
                region = MapRegion::MIRELANDS;
                location = TravelLocations::instance().Mirelands_Mirelands;
                camp = Camp::MIRELANDS_MIRELANDS;
                break;
            case 2:
                region = MapRegion::COASTLANDS;
                location = TravelLocations::instance().Coastlands_Beachside;
                camp = Camp::COASTLANDS_BEACHSIDE;
                break;
            case 3:
                region = MapRegion::HIGHLANDS;
                location = TravelLocations::instance().Highlands_Highlands;
                camp = Camp::HIGHLANDS_HIGHLANDS;
                break;
            case 4:
                region = MapRegion::ICELANDS;
                location = TravelLocations::instance().Icelands_Snowfields;
                camp = Camp::ICELANDS_SNOWFIELDS;
                break;
            }
        }
    }
    if (region == MapRegion::NONE){
        throw OperationFailedException(env.console, "Program internal error. No MMO region name found.");
    }

    env.log("Go to " + std::string(MAP_REGION_NAMES[int(region)]) + " to check MMO.");
    goto_camp_from_jubilife(env, env.console, context, location);

    // Open map
    pbf_press_button(context, BUTTON_MINUS, 50, 100);
    context.wait_for_all_requests();

    // Take a photo of the map before 
    QImage question_mark_image = env.console.video().snapshot();

    // Fix zoom level:
    const int zoom_level = read_map_zoom_level(question_mark_image);
    if (zoom_level < 0){
        throw OperationFailedException(env.console, "Canot read map zoom level.");
    }

    if (zoom_level == 0){
        pbf_press_button(context, BUTTON_ZR, 50, 50);
        context.wait_for_all_requests();
        question_mark_image = env.console.video().snapshot();
    } else if (zoom_level == 2){
        pbf_press_button(context, BUTTON_ZL, 50, 50);
        context.wait_for_all_requests();
        question_mark_image = env.console.video().snapshot();
    }

    // Move cursor away so that it does not show a text box that occludes MMO sprites.
    pbf_move_left_joystick(context, 0, 0, 300, 30);
    context.wait_for_all_requests();

    // Fix Missions & Requests tab:
    if (is_map_mission_tab_raised(question_mark_image)){
        pbf_press_button(context, BUTTON_R, 50, 100);
        context.wait_for_all_requests();
        question_mark_image = env.console.video().snapshot();
    }

    // Now detect question marks:
    MMOQuestionMarkDetector question_mark_detector(env.logger());

    const auto quest_results = question_mark_detector.detect_MMOs_on_region_map(question_mark_image);
    env.log("Detected MMO question marks:");
    for(const auto& box : quest_results){
        std::ostringstream os;
        os << "- " << box.center_x() << ", " << box.center_y() << " " << box.width() << " x " << box.height();
        env.log(os.str());
    }

    // Clean the detected boxes, make them square.
    std::vector<ImagePixelBox> new_boxes;
    for (size_t i = 0; i < quest_results.size(); i++){
        const auto& box = quest_results[i];

        int radius = int((box.width() + box.height()) / 4 + 0.5);
        int center_x = box.center_x();
        int center_y = box.center_y();
        auto new_box = ImagePixelBox(center_x - radius, center_y - radius, center_x + radius, center_y + radius);
        new_boxes.push_back(new_box);
    }

    // Leave map view, back to overworld
    pbf_press_button(context, BUTTON_B, 20, 50);

    // Now go to Mai to see the reviewed map
    goto_Mai_from_camp(env.logger(), context, camp);

    pbf_mash_button(context, BUTTON_A, 350);
    context.wait_for_all_requests();
    
    DialogueYellowArrowDetector yellow_arrow_detector(env.logger(), env.console.overlay(), true);
    int ret = wait_until(env.console, context, std::chrono::seconds(10), {{yellow_arrow_detector}});
    if (ret < 0){
        dump_image(env.console.logger(), env.program_info(), "DialogBoxNotDetected", env.console.video().snapshot().frame);
        throw OperationFailedException(env.console, "Dialog box not detected when waiting for MMO map.");
    }

    pbf_press_button(context, BUTTON_B, 50, 50);

    MapDetector map_detector;
    ret = wait_until(env.console, context, std::chrono::seconds(5), {{map_detector}});
    if (ret < 0){
        dump_image(env.console.logger(), env.program_info(), "MapNotDetected", env.console.video().snapshot().frame);
        throw OperationFailedException(env.console, "Map not detected after talking to Mai.");
    }
    env.console.log("Found revealed map thanks to Munchlax!");

    VideoOverlaySet mmo_sprites_overlay(env.console);
    for (size_t i = 0; i < new_boxes.size(); i++){
        mmo_sprites_overlay.add(COLOR_BLUE, pixelbox_to_floatbox(question_mark_image, new_boxes[i]));
    }

    // Move cursor away so that it does not show a text box that occludes MMO sprites.
    pbf_move_left_joystick(context, 0, 0, 300, 30);
    context.wait_for_all_requests();

    std::set<std::string> found;

    // Check MMO results:
    std::vector<std::string> sprites;
    QImage sprites_screen = env.console.video().snapshot();
    for (size_t i = 0; i < new_boxes.size(); i++){
        auto result = match_sprite_on_map(sprites_screen, new_boxes[i], region);
        env.console.log("Found sprite " + result.slug);
        stats.mmo_pokemon++;

        sprites.push_back(result.slug);
        if (desired_MMOs.find(result.slug) != desired_MMOs.end()){
            found.insert(result.slug);
        }
    }

    // Check star MMO results:
    std::vector<ImagePixelBox> star_boxes;
    for (size_t i = 0; i < new_boxes.size(); i++){
        const auto& sprite_box = new_boxes[i];
        int radius = sprite_box.width() / 2;
        int center_x = sprite_box.center_x();
        int center_y = sprite_box.center_y();
        ImagePixelBox star_box(center_x + radius/10, center_y - radius*16/10, center_x + radius * 5/4, center_y);
        star_boxes.push_back(std::move(star_box));
    }

    MMOSpriteStarSymbolDetector star_detector(sprites_screen, star_boxes);

    ret = wait_until(env.console, context, std::chrono::seconds(5), {{star_detector}});
    for (size_t i = 0; i < new_boxes.size(); i++){
        if (star_detector.is_star(i)){
            stats.stars++;
            env.log("Sprite " + sprites[i] + " has star.");
            if (desired_star_MMOs.find(sprites[i]) != desired_star_MMOs.end()){
                found.insert(sprites[i]);
            }
        }
    }

    env.update_stats();

    return found;
}


bool OutbreakFinder::run_iteration(
    SingleSwitchProgramEnvironment& env, BotBaseContext& context,
    const std::set<std::string>& desired_hisui_map_events,
    const std::set<std::string>& desired_outbreaks,
    const std::set<std::string>& desired_MMOs,
    const std::set<std::string>& desired_star_MMOs
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
    std::set<std::string> found_hisui_map_events = read_outbreaks(env, context, current_region, desired_hisui_map_events);
    if (current_region == MapRegion::NONE){
        stats.errors++;
        throw OperationFailedException(env.console, "Unable to read map.");
    }

    if (found_hisui_map_events.size() > 0){

        // Check if we found any Massive Outbreak pokemon (including MMO symbol targets)
        int num_outbreak_pokemon_found = 0;
        for(const auto& found : found_hisui_map_events){
            num_outbreak_pokemon_found += (desired_outbreaks.find(found) != desired_outbreaks.end());
        }
        if (num_outbreak_pokemon_found > 0){
            stats.matches += num_outbreak_pokemon_found;
            return true;
        }

        // What we found is MMO symbols for MMO pokemon.
        // Go into those regions to check MMO details.
        
        // Cancel map view
        pbf_press_button(context, BUTTON_B, 50, 50);
        // Leave the guard.
        pbf_move_left_joystick(context, 128, 0, 100, 50);
        // Checking MMO costs Aguav berries.
        // To not waste them, save here so that we can reset to get berries back.
        save_game_from_overworld(env, env.console, context);

        for(const auto& mmo_name: found_hisui_map_events){
            std::set<std::string> found_pokemon = read_MMOs(env, context, mmo_name, desired_MMOs, desired_star_MMOs);
            if (found_pokemon.size() > 0){
                stats.matches += found_pokemon.size();
                return true;
            }

            env.log("No target MMO sprite found. Reset game...");
            pbf_press_button(context, BUTTON_HOME, 20, GameSettings::instance().GAME_TO_HOME_DELAY);
            reset_game_from_home(env, env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
        }
    }

    env.update_stats();
    send_program_status_notification(env, NOTIFICATION_STATUS);

    //  Go to region and return.
    env.log("Go to a random region and back to refresh outbreaks...");
    goto_region_and_return(env, context, current_region);

    return false;
}



void OutbreakFinder::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    Stats& stats = env.current_stats<Stats>();

    // goto_Mai_from_camp(env.logger(), context, Camp::HIGHLANDS_HIGHLANDS);
    // return;


    auto to_set = [&](const Pokemon::PokemonNameList& name_list){
        std::set<std::string> ret;
        for (size_t c = 0; c < name_list.size(); c++){
            ret.insert(name_list[c]);
        }
        return ret;
    };

    std::set<std::string> desired_outbreaks = to_set(DESIRED_MO_SLUGS);
    std::set<std::string> desired_MMOs = to_set(DESIRED_MMO_SLUGS);
    std::set<std::string> desired_star_MMOs = to_set(DESIRED_STAR_MMO_SLUGS);

    std::set<std::string> desired_hisui_map_events = desired_outbreaks;
    std::map<std::string, int> MMO_targets;
    for(size_t i = 0; i < 5; i++){
        for(const std::string& sprite_slug: MMO_FIRST_WAVE_REGION_SPRITE_SLUGS()[i]){
            if (desired_MMOs.find(sprite_slug) != desired_MMOs.end() || desired_star_MMOs.find(sprite_slug) != desired_star_MMOs.end()){
                MMO_targets[MMO_NAMES().name_list[i]]++;
            }
        }
    }

    std::ostringstream os;
    os << "User requires MMO pokemon. Need to visit ";
    for(const auto& p : MMO_targets){
        os << "(" << p.first << " " << p.second << ") ";
    }
    env.log(os.str());

    for(const auto& p : MMO_targets){
        desired_hisui_map_events.insert(p.first);
    }


    while (true){
        if (run_iteration(env, context, desired_hisui_map_events, desired_outbreaks, desired_MMOs, desired_star_MMOs)){
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
