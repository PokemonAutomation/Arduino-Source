/*  Pokemon LA MMO Routines
 *
 *  Functions to run MMO related tasks
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonLA/Inference/Map/PokemonLA_PokemonMapSpriteReader.h"
#include "PokemonLA/Inference/Map/PokemonLA_MapDetector.h"
#include "PokemonLA/Inference/Map/PokemonLA_MapMissionTabReader.h"
#include "PokemonLA/Inference/Map/PokemonLA_MapZoomLevelReader.h"
#include "PokemonLA/Inference/Map/PokemonLA_MMOSpriteStarSymbolDetector.h"
#include "PokemonLA/Inference/Objects/PokemonLA_MMOQuestionMarkDetector.h"
#include "PokemonLA/Inference/PokemonLA_DialogDetector.h"
#include "PokemonLA/PokemonLA_Locations.h"
#include "PokemonLA/PokemonLA_TravelLocations.h"
#include "PokemonLA/Programs/PokemonLA_RegionNavigation.h"
#include <sstream>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

    
std::vector<std::string> load_mmo_names(){
    return {
        "fieldlands-mmo",
        "mirelands-mmo",
        "coastlands-mmo",
        "highlands-mmo",
        "icelands-mmo"
    };
}

const std::vector<std::string>& MMO_NAMES(){
    const static std::vector<std::string> mmo_names = load_mmo_names();
    return mmo_names;
}


std::set<std::string> enter_region_and_read_MMO(
    SingleSwitchProgramEnvironment& env, ProControllerContext& context,
    const std::string& mmo_name,
    const std::set<std::string>& desired_MMOs,
    const std::set<std::string>& desired_star_MMOs,
    bool debug_mode,
    int& num_mmo_pokemon_found,
    int& num_star_mmo_found,
    bool fresh_from_reset
){
    MapRegion region = MapRegion::NONE;
    TravelLocation location = TravelLocations::instance().Fieldlands_Fieldlands;
    Camp camp = Camp::FIELDLANDS_FIELDLANDS;
    // When you open the map in a region, the map cursor is initialized at your current location
    // on map. But when you land in a region, the initial location is a camp, the map cursor
    // will show a text box "xxx Camp - Take a rest or do some crafting". This may occlude some
    // MMO question marks. So we have to move the map cursor away after we land in the region.
    uint8_t map_cursor_move_x = 0;
    uint8_t map_cursor_move_y = 0;
    const int map_cursor_move_duration = 50;

    for(size_t i = 0; i < 5; i++){
        if (mmo_name == MMO_NAMES()[i]){
            switch (i){
            case 0:
                region = MapRegion::FIELDLANDS;
                location = TravelLocations::instance().Fieldlands_Fieldlands;
                camp = Camp::FIELDLANDS_FIELDLANDS;
                map_cursor_move_x = 128;
                map_cursor_move_y = 0;
                break;
            case 1:
                region = MapRegion::MIRELANDS;
                location = TravelLocations::instance().Mirelands_Mirelands;
                camp = Camp::MIRELANDS_MIRELANDS;
                map_cursor_move_x = 0;
                map_cursor_move_y = 128;
                break;
            case 2:
                region = MapRegion::COASTLANDS;
                location = TravelLocations::instance().Coastlands_Beachside;
                camp = Camp::COASTLANDS_BEACHSIDE;
                map_cursor_move_x = 0;
                map_cursor_move_y = 128;
                break;
            case 3:
                region = MapRegion::HIGHLANDS;
                location = TravelLocations::instance().Highlands_Highlands;
                camp = Camp::HIGHLANDS_HIGHLANDS;
                map_cursor_move_x = 128;
                map_cursor_move_y = 255;
                break;
            case 4:
                region = MapRegion::ICELANDS;
                location = TravelLocations::instance().Icelands_Snowfields;
                camp = Camp::ICELANDS_SNOWFIELDS;
                map_cursor_move_x = 128;
                map_cursor_move_y = 255;
                break;
            }
        }
    }
    if (region == MapRegion::NONE){
        throw InternalProgramError(&env.console.logger(), PA_CURRENT_FUNCTION, "No MMO region name found.");
    }

    env.log("Go to " + std::string(MAP_REGION_NAMES[int(region)]) + " to check MMO.");
    goto_camp_from_jubilife(env, env.console, context, location, fresh_from_reset);

    // Open map
    pbf_press_button(context, BUTTON_MINUS, 50, 100);
    context.wait_for_all_requests();

    // Take a photo of the map before 
    VideoSnapshot question_mark_image = env.console.video().snapshot();

    // Fix zoom level:
    const int zoom_level = read_map_zoom_level(question_mark_image);
    if (zoom_level < 0){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Canot read map zoom level.",
            env.console
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
    pbf_move_left_joystick(context, map_cursor_move_x, map_cursor_move_y, map_cursor_move_duration, 30);
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
                ErrorReport::SEND_ERROR_REPORT,
                "Dialog box not detected when waiting for MMO map.",
                env.console
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
                ErrorReport::SEND_ERROR_REPORT,
                "Map not detected after talking to Mai.",
                env.console
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
    pbf_move_left_joystick(context, map_cursor_move_x, map_cursor_move_y, map_cursor_move_duration, 30);
    context.wait_for_all_requests();

    std::set<std::string> found;

    // Check MMO results:
    std::vector<std::string> sprites;
    VideoSnapshot sprites_screen = env.console.video().snapshot();
    for (size_t i = 0; i < new_boxes.size(); i++){
        auto result = match_sprite_on_map(env.logger(), sprites_screen, new_boxes[i], region, debug_mode);
        env.console.log("Found MMO sprite " + result.slug);
        num_mmo_pokemon_found++;

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
            num_star_mmo_found++;
            os << ", has star";
            if (desired_star_MMOs.find(sprites[i]) != desired_star_MMOs.end()){
                found.insert(sprites[i]);
            }
        }
        env.log(os.str());
    }
    return found;
}



}
}
}
