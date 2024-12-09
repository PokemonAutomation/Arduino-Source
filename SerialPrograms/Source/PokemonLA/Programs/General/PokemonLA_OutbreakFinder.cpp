/*  Outbreak Finder
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <iterator>
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
//#include "CommonFramework/ImageMatch/ImageCropper.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Resources/Pokemon_PokemonNames.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonLA/Inference/PokemonLA_DialogDetector.h"
#include "PokemonLA/Inference/Map/PokemonLA_MapDetector.h"
#include "PokemonLA/Inference/Map/PokemonLA_SelectedRegionDetector.h"
#include "PokemonLA/Inference/Map/PokemonLA_OutbreakReader.h"
#include "PokemonLA/Inference/Map/PokemonLA_MapMissionTabReader.h"
#include "PokemonLA/Inference/Map/PokemonLA_MapZoomLevelReader.h"
#include "PokemonLA/Inference/Map/PokemonLA_MMOSpriteStarSymbolDetector.h"
#include "PokemonLA/Inference/Map/PokemonLA_PokemonMapSpriteReader.h"
#include "PokemonLA/Programs/PokemonLA_GameEntry.h"
#include "PokemonLA/Programs/PokemonLA_GameSave.h"
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



StringSelectDatabase make_mo_database(){
    const SpriteDatabase& sprites = ALL_POKEMON_SPRITES();
    const std::vector<std::string>& slugs = HISUI_OUTBREAK_SLUGS();

    static const ImageRGB32 mmo_sprite(RESOURCE_PATH() + "PokemonLA/MMOQuestionMark-Template.png");

    StringSelectDatabase ret;
    ret.add_entry(StringSelectEntry("fieldlands-mmo",   "Fieldlands MMO",   mmo_sprite));
    ret.add_entry(StringSelectEntry("mirelands-mmo",    "Mirelands MMO",    mmo_sprite));
    ret.add_entry(StringSelectEntry("coastlands-mmo",   "Coastlands MMO",   mmo_sprite));
    ret.add_entry(StringSelectEntry("highlands-mmo",    "Highlands MMO",    mmo_sprite));
    ret.add_entry(StringSelectEntry("icelands-mmo",     "Icelands MMO",     mmo_sprite));
    for (const std::string& slug : slugs){
        const PokemonNames& name = get_pokemon_name(slug);
        const SpriteDatabase::Sprite& sprite = sprites.get_throw(slug);
        ret.add_entry(StringSelectEntry(
            slug, name.display_name(),
            sprite.icon
        ));
    }
    return ret;
}
const StringSelectDatabase& MO_DATABASE(){
    static const StringSelectDatabase database = make_mo_database();
    return database;
}

StringSelectDatabase make_mmo_database(){
    const SpriteDatabase& sprites = ALL_MMO_SPRITES();
    const std::vector<std::string>& slugs = MMO_FIRST_WAVE_SPRITE_SLUGS();
    std::vector<std::string> displays = load_pokemon_slug_json_list("PokemonLA/MMOFirstWaveSpriteNameDisplay.json");

    if (slugs.size() != displays.size()){
        throw FileException(
            nullptr, PA_CURRENT_FUNCTION,
            "Name and slug lists are not the same size.",
            "PokemonLA/MMOFirstWaveSpriteNameDisplay.json"
        );
    }

    StringSelectDatabase ret;
    for (size_t c = 0; c < displays.size(); c++){
        ret.add_entry(StringSelectEntry(
            slugs[c], std::move(displays[c]),
            sprites.get_throw(slugs[c]).icon
        ));
    }
    return ret;
}
const StringSelectDatabase& MMO_DATABASE(){
    static const StringSelectDatabase database = make_mmo_database();
    return database;
}



namespace{



std::vector<std::string> load_mmo_names(){
    return {
        "fieldlands-mmo",
        "mirelands-mmo",
        "coastlands-mmo",
        "highlands-mmo",
        "icelands-mmo"
    };
}

// The name of each MMO event happening at each region. Their slugs are:
// - "fieldlands-mmo"
// - "mirelands-mmo"
// - "coastlands-mmo"
// - "highlands-mmo"
// - "icelands-mmo"
const std::vector<std::string>& MMO_NAMES(){
    const static std::vector<std::string> mmo_names = load_mmo_names();
    return mmo_names;
}


} // anonymous namespace


OutbreakFinder_Descriptor::OutbreakFinder_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLA:OutbreakFinder",
        STRING_POKEMON + " LA", "Outbreak Finder",
        "ComputerControl/blob/master/Wiki/Programs/PokemonLA/OutbreakFinder.md",
        "Search for an outbreak for a specific " + STRING_POKEMON,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}
class OutbreakFinder_Descriptor::Stats : public StatsTracker{
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
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Outbreaks");
        m_display_order.emplace_back("MMOs");
        m_display_order.emplace_back("MMO Pokemon");
        m_display_order.emplace_back("Stars");
        m_display_order.emplace_back("Matches", HIDDEN_IF_ZERO);
    }

    std::atomic<uint64_t>& checks;
    std::atomic<uint64_t>& errors;
    std::atomic<uint64_t>& outbreaks;
    std::atomic<uint64_t>& mmos;
    std::atomic<uint64_t>& mmo_pokemon;
    std::atomic<uint64_t>& stars; // MMO star symbols
    std::atomic<uint64_t>& matches;
};
std::unique_ptr<StatsTracker> OutbreakFinder_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}





OutbreakFinder::OutbreakFinder()
    : GO_HOME_WHEN_DONE(false)
    , LANGUAGE(
        "<b>Game Language:</b>",
        Pokemon::PokemonNameReader::instance().languages(),
        LockMode::LOCK_WHILE_RUNNING,
        true
    )
    , DESIRED_MO_SLUGS(
        "<b>Desired Outbreak " + STRING_POKEMON + ":</b><br>Stop when anything on this list is found.",
        STRING_POKEMON, MO_DATABASE(), "cherubi"
    )
    , DESIRED_MMO_SLUGS(
        "<b>Desired first MMO wave " + STRING_POKEMON + ":</b><br>Stop when anything on this list is found.",
        STRING_POKEMON, MMO_DATABASE(), "rowlet"
    )
    , DESIRED_STAR_MMO_SLUGS(
        "<b>Desired first MMO wave " + STRING_POKEMON + " with shiny symbols:</b><br>Stop when anything on this list is found.",
        STRING_POKEMON, MMO_DATABASE(), "rowlet"
    )
    , DEBUG_MODE(
        "<b>Debug Mode:</b><br>Save MMO Sprite to debug folder.",
        LockMode::LOCK_WHILE_RUNNING,
        false
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

    if (PreloadSettings::instance().DEVELOPER_MODE){
        PA_ADD_OPTION(DEBUG_MODE);
    }
}


std::set<std::string> OutbreakFinder::read_travel_map_outbreaks(
    SingleSwitchProgramEnvironment& env, BotBaseContext& context,
    const std::set<std::string>& desired_events
){
    OutbreakFinder_Descriptor::Stats& stats = env.current_stats<OutbreakFinder_Descriptor::Stats>();

    MapRegion start_region = MapRegion::NONE;

    MapRegion current_region = detect_selected_region(env.console, context);
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
            auto iter = desired_events.find(mmo_names[i]);
            if (iter != desired_events.end()){
                env.console.log("Found a match!", COLOR_BLUE);
                found.insert(mmo_names[i]);
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
            }else{
                OutbreakReader reader(env.console, LANGUAGE, env.console);
                OCR::StringMatchResult result = reader.read(env.console.video().snapshot());
                if (!result.results.empty()){
                    stats.outbreaks++;
                }
                for (const auto& item : result.results){
                    auto iter = desired_events.find(item.second.token);
                    if (iter != desired_events.end()){
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

    return found;
}


void OutbreakFinder::goto_region_and_return(SingleSwitchProgramEnvironment& env, BotBaseContext& context, 
    bool inside_travel_map
){
    OutbreakFinder_Descriptor::Stats& stats = env.current_stats<OutbreakFinder_Descriptor::Stats>();

    if (inside_travel_map == false){
        // Move to guard to open map
        open_travel_map_from_jubilife(env, env.console, context);
        context.wait_for(std::chrono::milliseconds(500));
    }

    MapRegion current_region = MapRegion::NONE;
    for (size_t c = 0; c < 10; c++){
        current_region = detect_selected_region(env.console, context);
        if (is_wild_land(current_region)){
            break;
        }

        if (current_region == MapRegion::JUBILIFE){
            // Move to fieldlands
            pbf_press_dpad(context, DPAD_RIGHT, 20, 40);
        }else if (current_region == MapRegion::RETREAT){
            // Move to icelands
            pbf_press_dpad(context, DPAD_LEFT, 20, 40);
        }else{
            // Cannot read current region. Try move to another region
            pbf_press_dpad(context, DPAD_RIGHT, 20, 40);
        }
        context.wait_for_all_requests();
    }
    if (is_wild_land(current_region) == false){
        dump_image(env.console.logger(), env.program_info(), "FindRegion", env.console.video().snapshot());
        OperationFailedException::fire(
            env.console, ErrorReport::SEND_ERROR_REPORT,
            "Unable to find a wild land."
        );
    }

    mash_A_to_change_region(env, env.console, context);

    Camp camp = Camp::FIELDLANDS_FIELDLANDS;
    switch (current_region){
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

std::set<std::string> OutbreakFinder::enter_region_and_read_MMO(
    SingleSwitchProgramEnvironment& env, BotBaseContext& context,
    const std::string& mmo_name,
    const std::set<std::string>& desired_MMOs,
    const std::set<std::string>& desired_star_MMOs
){
    OutbreakFinder_Descriptor::Stats& stats = env.current_stats<OutbreakFinder_Descriptor::Stats>();

    MapRegion region = MapRegion::NONE;
    TravelLocation location = TravelLocations::instance().Fieldlands_Fieldlands;
    Camp camp = Camp::FIELDLANDS_FIELDLANDS;
    for(size_t i = 0; i < 5; i++){
        if (mmo_name == MMO_NAMES()[i]){
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
        throw InternalProgramError(&env.console.logger(), PA_CURRENT_FUNCTION, "No MMO region name found.");
    }

    env.log("Go to " + std::string(MAP_REGION_NAMES[int(region)]) + " to check MMO.");
    goto_camp_from_jubilife(env, env.console, context, location);

    // Open map
    pbf_press_button(context, BUTTON_MINUS, 50, 100);
    context.wait_for_all_requests();

    // Take a photo of the map before 
    VideoSnapshot question_mark_image = env.console.video().snapshot();

    // Fix zoom level:
    const int zoom_level = read_map_zoom_level(question_mark_image);
    if (zoom_level < 0){
        OperationFailedException::fire(
            env.console, ErrorReport::SEND_ERROR_REPORT,
            "Canot read map zoom level."
        );
    }

    if (zoom_level == 0){
        pbf_press_button(context, BUTTON_ZR, 50, 50);
        context.wait_for_all_requests();
        question_mark_image = env.console.video().snapshot();
    }else if (zoom_level == 2){
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

        pxint_t radius = (pxint_t)((box.width() + box.height()) / 4 + 0.5);
        pxint_t center_x = (pxint_t)box.center_x();
        pxint_t center_y = (pxint_t)box.center_y();
        auto new_box = ImagePixelBox(center_x - radius, center_y - radius, center_x + radius, center_y + radius);
        new_boxes.push_back(new_box);
    }

    // Leave map view, back to overworld
    pbf_press_button(context, BUTTON_B, 20, 50);

    // Now go to Mai to see the reviewed map
    goto_Mai_from_camp(env.logger(), context, camp);

    pbf_mash_button(context, BUTTON_A, 350);
    context.wait_for_all_requests();

    // Wait for the last dialog box before the MMO pokemon sprites are revealed.
    {
        EventDialogDetector event_dialog_detector(env.logger(), env.console.overlay(), true);
        int ret = wait_until(env.console, context, std::chrono::seconds(10), {{event_dialog_detector}});
        if (ret < 0){
            OperationFailedException::fire(
                env.console, ErrorReport::SEND_ERROR_REPORT,
                "Dialog box not detected when waiting for MMO map."
            );
        }
    }
    pbf_press_button(context, BUTTON_B, 50, 50);

    while (true){
        EventDialogDetector event_dialog_detector(env.logger(), env.console.overlay(), true);
        MapDetector map_detector;
        context.wait_for_all_requests();
        int ret = wait_until(
            env.console, context, std::chrono::seconds(10),
            {event_dialog_detector, map_detector}
        );
        switch (ret){
        case 0:
            env.console.log("Detected dialog.");
            pbf_press_button(context, BUTTON_B, 20, 105);
            continue;
        case 1:
            env.console.log("Found revealed map thanks to Munchlax!");
            break;
        default:
            OperationFailedException::fire(
                env.console, ErrorReport::SEND_ERROR_REPORT,
                "Map not detected after talking to Mai."
            );
        }
        break;
    }

#if 0
    MapDetector map_detector;
    ret = wait_until(env.console, context, std::chrono::seconds(5), {{map_detector}});
    if (ret < 0){
        OperationFailedException::fire(
            env.console, ErrorReport::SEND_ERROR_REPORT,
            "Map not detected after talking to Mai.",
            true
        );
    }
    env.console.log("Found revealed map thanks to Munchlax!");
#endif

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
    VideoSnapshot sprites_screen = env.console.video().snapshot();
    for (size_t i = 0; i < new_boxes.size(); i++){
        auto result = match_sprite_on_map(env.logger(), sprites_screen, new_boxes[i], region, DEBUG_MODE);
        env.console.log("Found MMO sprite " + result.slug);
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
        pxint_t radius = (pxint_t)sprite_box.width() / 2;
        pxint_t center_x = (pxint_t)sprite_box.center_x();
        pxint_t center_y = (pxint_t)sprite_box.center_y();
        ImagePixelBox star_box(
            center_x + radius/10,
            center_y - radius*16/10,
            center_x + radius * 5/4,
            center_y
        );
        star_boxes.push_back(std::move(star_box));
    }

    MMOSpriteStarSymbolDetector star_detector(sprites_screen, star_boxes);

    env.log("Detect star symbols...");
    wait_until(env.console, context, std::chrono::seconds(5), {{star_detector}});
    for (size_t i = 0; i < new_boxes.size(); i++){
        std::ostringstream os;
        os << "- " << sprites[i] << " box [" << star_boxes[i].min_x << ", " << star_boxes[i].min_y
           << star_boxes[i].max_x << ", " << star_boxes[i].max_y << "]"
           <<  " motion: " << star_detector.animation_value(i)
           << " color: " << star_detector.symbol_color(i);
        if (star_detector.is_star(i)){
            stats.stars++;
            os << ", has star";
            if (desired_star_MMOs.find(sprites[i]) != desired_star_MMOs.end()){
                found.insert(sprites[i]);
            }
        }
        env.log(os.str());
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
    OutbreakFinder_Descriptor::Stats& stats = env.current_stats<OutbreakFinder_Descriptor::Stats>();

    //  Enter map.
    try{
        open_travel_map_from_jubilife(env, env.console, context);
    }catch (OperationFailedException&){
        stats.errors++;
        throw;
    }
    context.wait_for(std::chrono::milliseconds(500));

    // found_hisui_map_events: desired pokemon names or MMO names
    std::set<std::string> found_hisui_map_events;
    found_hisui_map_events = read_travel_map_outbreaks(env, context, desired_hisui_map_events);

    bool inside_travel_map = true;
    if (found_hisui_map_events.size() > 0){
        // Check if we found any Massive Outbreak pokemon (including MMO symbol targets)
        {
            std::vector<std::string> desired_outbreaks_found;
            for(const auto& found : found_hisui_map_events){
                if (desired_outbreaks.find(found) != desired_outbreaks.end()){
                    desired_outbreaks_found.push_back(found);
                }
            }
            if (desired_outbreaks_found.size() > 0){
                stats.matches += desired_outbreaks_found.size();
                std::ostringstream os;
                os << "Found following desired outbreak" << (desired_outbreaks_found.size() > 1 ? "s: " : ": ");
                for(const auto& outbreak: desired_outbreaks_found){
                    os << outbreak << ", ";
                }
                env.log(os.str());
                return true;
            }else{
                env.log("No desired outbreak.");
            }
        }

        // What we found is MMO symbols for MMO pokemon.
        // Go into those regions to check MMO details.

        // Cancel map view
        inside_travel_map = false;
        pbf_press_button(context, BUTTON_B, 50, 50);
        // Leave the guard.
        pbf_move_left_joystick(context, 128, 0, 100, 50);
        // Checking MMO costs Aguav berries.
        // To not waste them, save here so that we can reset to get berries back.
        save_game_from_overworld(env, env.console, context);

        for(const auto& mmo_name: found_hisui_map_events){
            std::set<std::string> found_pokemon = enter_region_and_read_MMO(env, context, mmo_name, desired_MMOs, desired_star_MMOs);
            if (found_pokemon.size() > 0){
                stats.matches += found_pokemon.size();
                std::ostringstream os;
                os << "Found desired MMO pokemon (including desired MMO pokemon with star symbols): ";
                for (const auto& pokemon : found_pokemon){
                    os << pokemon << ", ";
                }
                env.log(os.str());
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
    goto_region_and_return(env, context, inside_travel_map);

    return false;
}


std::set<std::string> OutbreakFinder::to_set(const StringSelectTableOption& option){
    std::set<std::string> ret;
    for (std::string& slug : option.all_slugs()){
        ret.insert(std::move(slug));
    }
    return ret;
}

void OutbreakFinder::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
//    OutbreakFinder_Descriptor::Stats& stats = env.current_stats<OutbreakFinder_Descriptor::Stats>();

    // goto_Mai_from_camp(env.logger(), context, Camp::HIGHLANDS_HIGHLANDS);
    // return;

    std::set<std::string> desired_outbreaks = to_set(DESIRED_MO_SLUGS);
    std::set<std::string> desired_MMO_pokemon = to_set(DESIRED_MMO_SLUGS);
    std::set<std::string> desired_star_MMO_pokemon = to_set(DESIRED_STAR_MMO_SLUGS);
    {
        std::ostringstream os;
        os << "Desired outbreaks: ";
        std::copy(desired_outbreaks.begin(), desired_outbreaks.end(), std::ostream_iterator<std::string>(os, ", "));
        env.log(os.str());
        os = std::ostringstream();
        os << "Desired first-wave MMO pokemon: ";
        std::copy(desired_MMO_pokemon.begin(), desired_MMO_pokemon.end(), std::ostream_iterator<std::string>(os, ", "));
        env.log(os.str());
        os = std::ostringstream();
        os << "Desired MMO pokemon with star: ";
        std::copy(desired_star_MMO_pokemon.begin(), desired_star_MMO_pokemon.end(), std::ostream_iterator<std::string>(os, ", "));
        env.log(os.str());
    }

    // desired_hisui_map_events: the slugs of travel map event. This includes
    // - Massive Outbreak pokemon
    // - Question mark MMO event at any region
    std::set<std::string> desired_hisui_map_events = desired_outbreaks;

    // If the user sets MMO pokemon, then we should add the MMO map events that may spawn those MMO pokemon to
    // `desired_hisui_map_events`.

    // MMO_targets: MMO map event slug (e.g. "fieldlands-mmo") -> how many desired MMO pokemon can spawn on this map
    std::map<std::string, int> MMO_targets;
    // Check if each MMO map event may spawn desired MMO pokemon:
    for(size_t i = 0; i < 5; i++){
        for(const std::string& sprite_slug: MMO_FIRST_WAVE_REGION_SPRITE_SLUGS()[i]){
            if (desired_MMO_pokemon.find(sprite_slug) != desired_MMO_pokemon.end()
                || desired_star_MMO_pokemon.find(sprite_slug) != desired_star_MMO_pokemon.end()
            ){
                MMO_targets[MMO_NAMES()[i]]++;
            }
        }
    }

    std::ostringstream os;
    os << "User requires MMO pokemon. Need to visit (\"map MMO name\", \"how many desired MMO pokemon on the map\"): ";
    for(const auto& p : MMO_targets){
        os << "(" << p.first << ", " << p.second << ") ";
    }
    env.log(os.str());

    // Add MMO map event targets (e.g. "fieldlands-mmo") derived from user selected MMO pokemon to 
    // `desired_hisui_map_events`.
    for(const auto& p : MMO_targets){
        desired_hisui_map_events.insert(p.first);
    }

    while (true){
        if (run_iteration(env, context, desired_hisui_map_events, desired_outbreaks, desired_MMO_pokemon, 
            desired_star_MMO_pokemon))
        {
            break;
        }
    }

    env.update_stats();

    send_program_notification(
        env, NOTIFICATION_MATCHED,
        COLOR_GREEN,
        "Found Outbreak",
        {}, "",
        env.console.video().snapshot()
    );

    GO_HOME_WHEN_DONE.run_end_of_program(context);
}




}
}
}
