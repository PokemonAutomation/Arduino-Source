/*  Home Box Navigation
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <array>
#include <optional>
#include <sstream>
#include <vector>
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/Notifications/ProgramInfo.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/OCR/OCR_NumberReader.h"
#include "CommonTools/VisualDetectors/FrozenImageDetector.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Resources/Pokemon_PokemonSlugs.h"
#include "Pokemon/Inference/Pokemon_TypeReader.h"
#include "PokemonHome/Inference/PokemonHome_BallReader.h"
#include "PokemonHome/Inference/PokemonHome_BoxGenderDetector.h"
#include "PokemonHome_BoxNavigation.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{
using namespace Pokemon;


// Move the red cursor to the first slot of the box
// If the cursor is not add the first slot, move the cursor to the left and up one row at a time until it is at the first slot. 
bool go_to_first_slot(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    Milliseconds VIDEO_DELAY
) {
    ImageFloatBox first_slot_cursor_box(0.07, 0.15, 0.01, 0.01); //cursor position of the first slot of the box
    VideoSnapshot screen = env.console.video().snapshot();
    FloatPixel first_slot_cursor_color = image_stats(extract_box_reference(screen, first_slot_cursor_box)).average;
    env.console.log("BoxCursor color detection: " + first_slot_cursor_color.to_string());
    VideoOverlaySet BoxRender(env.console);
    BoxRender.add(COLOR_BLUE, first_slot_cursor_box);

    // If the cursor is not at the first slot
    if (first_slot_cursor_color.r <= first_slot_cursor_color.g + first_slot_cursor_color.b) {
        bool cursor_found = false;
        for (uint8_t rows = 0; rows < 7; rows++) {
            for (uint8_t column = 0; column < 5; column++) {
                pbf_press_dpad(context, DPAD_LEFT, 80ms, VIDEO_DELAY);
                context.wait_for_all_requests();
                screen = env.console.video().snapshot();
                first_slot_cursor_color = image_stats(extract_box_reference(screen, first_slot_cursor_box)).average;
                env.console.log("BoxCursor color detection: " + first_slot_cursor_color.to_string());

                if (first_slot_cursor_color.r > first_slot_cursor_color.g + first_slot_cursor_color.b) {
                    cursor_found = true;
                    break;
                }
            }
            if (!cursor_found) {
                pbf_press_dpad(context, DPAD_UP, 80ms, VIDEO_DELAY);
                context.wait_for_all_requests();
                screen = env.console.video().snapshot();
                first_slot_cursor_color = image_stats(extract_box_reference(screen, first_slot_cursor_box)).average;
                env.console.log("BoxCursor color detection: " + first_slot_cursor_color.to_string());

                if (first_slot_cursor_color.r > first_slot_cursor_color.g + first_slot_cursor_color.b) {
                    cursor_found = true;
                    break;
                }
            }
            else {
                break;
            }
        }
        if (!cursor_found) {
            return false;
        }
    }
    BoxRender.clear();
    return true;
}

//Move the cursor to the given coordinates, knowing current pos via the cursor struct
[[nodiscard]] BoxCursor move_cursor_to(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    const BoxCursor& cur_cursor,
    const BoxCursor& dest_cursor,
    Milliseconds GAME_DELAY
) {

    std::ostringstream ss;
    ss << "Moving cursor from " << cur_cursor << " to " << dest_cursor;
    env.console.log(ss.str());

    // TODO: shortest path movement though pages, boxes
    for (size_t i = cur_cursor.box; i < dest_cursor.box; ++i) {
        pbf_press_button(context, BUTTON_R, 80ms, GAME_DELAY + 240ms);
    }
    for (size_t i = dest_cursor.box; i < cur_cursor.box; ++i) {
        pbf_press_button(context, BUTTON_L, 80ms, GAME_DELAY + 240ms);
    }


    // direct nav up or down through rows
    if (!(cur_cursor.row == 0 && dest_cursor.row == 4) && !(dest_cursor.row == 0 && cur_cursor.row == 4)) {
        for (size_t i = cur_cursor.row; i < dest_cursor.row; ++i) {
            pbf_press_dpad(context, DPAD_DOWN, 80ms, GAME_DELAY);
        }
        for (size_t i = dest_cursor.row; i < cur_cursor.row; ++i) {
            pbf_press_dpad(context, DPAD_UP, 80ms, GAME_DELAY);
        }
    }
    else { // wrap around is faster to move between first or last row
        if (cur_cursor.row == 0 && dest_cursor.row == 4) {
            for (size_t i = 0; i <= 2; ++i) {
                pbf_press_dpad(context, DPAD_UP, 80ms, GAME_DELAY);
            }
        }
        else {
            for (size_t i = 0; i <= 2; ++i) {
                pbf_press_dpad(context, DPAD_DOWN, 80ms, GAME_DELAY);
            }
        }
    }

    // direct nav forward or backward through columns
    if ((dest_cursor.column > cur_cursor.column && dest_cursor.column - cur_cursor.column <= 3) || (cur_cursor.column > dest_cursor.column && cur_cursor.column - dest_cursor.column <= 3)) {
        for (size_t i = cur_cursor.column; i < dest_cursor.column; ++i) {
            pbf_press_dpad(context, DPAD_RIGHT, 80ms, GAME_DELAY);
        }
        for (size_t i = dest_cursor.column; i < cur_cursor.column; ++i) {
            pbf_press_dpad(context, DPAD_LEFT, 80ms, GAME_DELAY);
        }
    }
    else { // wrap around is faster if direct movement is more than 3 away
        if (dest_cursor.column > cur_cursor.column) {
            for (size_t i = 0; i < BOX_COLS - (dest_cursor.column - cur_cursor.column); ++i) {
                pbf_press_dpad(context, DPAD_LEFT, 80ms, GAME_DELAY);
            }
        }
        if (cur_cursor.column > dest_cursor.column) {
            for (size_t i = 0; i < BOX_COLS - (cur_cursor.column - dest_cursor.column); ++i) {
                pbf_press_dpad(context, DPAD_RIGHT, 80ms, GAME_DELAY);
            }
        }
    }

    context.wait_for_all_requests();
    return dest_cursor;
}

// Read current screen to find occupied and empty slots in the box.
// Add a placeholder value for each slot in order into `boxes_data`. For empty slot the value is just std::nullopt, while
// for the occupied slot it is an empty pokemon struct `CollectedPokemonInfo`.
// Return the (row, col) index of the first pokemon (aka non-empty) slot in the box.
std::array<size_t, 2> find_occupied_slots_in_box(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    std::vector<std::optional<CollectedPokemonInfo>>& boxes_data,
    const std::vector<SortingRule>& sort_preferences)
{
    //BoxSorter_Descriptor::Stats& stats = env.current_stats< BoxSorter_Descriptor::Stats>();

    VideoSnapshot screen = env.console.video().snapshot();

    std::ostringstream ss;
    ss << "\n";

    std::array<size_t, 2> first_pokemon_slot = { SIZE_MAX, SIZE_MAX };

    int num_empty_slots = 0;
    for (size_t row = 0; row < BOX_ROWS; row++) {
        for (size_t col = 0; col < BOX_COLS; col++) {
            ImageFloatBox slot_box(0.06 + (0.072 * col), 0.2 + (0.1035 * row), 0.03, 0.057);
            int current_box_value = (int)image_stddev(extract_box_reference(screen, slot_box)).sum();

            ss << current_box_value;

            //checking color to know if a pokemon is on the slot or not
            if (current_box_value < 10) {
                //stats.empty++;
                num_empty_slots++;
                boxes_data.push_back(std::nullopt); //empty optional to make sorting easier later
                ss << "\u274c ";    //  "X"
            }
            else {
                if (first_pokemon_slot[0] == SIZE_MAX) {
                    first_pokemon_slot = { row, col };
                }
                //stats.pkmn++;
                boxes_data.push_back(
                    CollectedPokemonInfo{
                        .preferences = &sort_preferences
                    }
                ); //default initialised pokemon to know there is a pokemon here that needs a value
                ss << "\u2705 ";    //  checkbox
            }
        }
        ss << "\n";
    }

    //env.update_stats();
    env.log(ss.str());
    env.add_overlay_log("Empty: " + std::to_string(num_empty_slots) + "/30");

    return first_pokemon_slot;
}

// Read the current summary screen and assign various pokemon info into `cur_pokemon-info`
void read_summary_screen(
    SingleSwitchProgramEnvironment& env, ProControllerContext& context,
    CollectedPokemonInfo& cur_pokemon_info
) {
    VideoOverlaySet video_overlay_set(env.console);

    ImageFloatBox national_dex_number_box(0.448, 0.245, 0.049, 0.04); //pokemon national dex number pos
    ImageFloatBox shiny_symbol_box(0.702, 0.09, 0.04, 0.06); // shiny symbol pos
    // TODO: gmax symbol is at the same location as Tera type symbol! Need better detection to tell apart
    // gmax symbol and tera types
    ImageFloatBox gmax_symbol_box(0.463, 0.09, 0.04, 0.06); // gmax symbol pos
    ImageFloatBox origin_symbol_box(0.623, 0.095, 0.033, 0.05); // origin symbol pos
    ImageFloatBox pokemon_box(0.69, 0.18, 0.28, 0.46); // pokemon render pos
    ImageFloatBox level_box(0.546, 0.099, 0.044, 0.041); // Level
    ImageFloatBox ot_id_box(0.782, 0.719, 0.193, 0.046); // OT ID
    ImageFloatBox ot_box(0.492, 0.719, 0.165, 0.049); // OT
    ImageFloatBox nature_box(0.157, 0.783, 0.212, 0.042); // Nature
    ImageFloatBox ability_box(0.158, 0.838, 0.213, 0.042); // Ability
    ImageFloatBox alpha_box(0.787, 0.095, 0.024, 0.046); // Alpha symbol
    ImageFloatBox type_box(0.615, 0.240, 0.071, 0.057); // Type symbols


    video_overlay_set.add(COLOR_WHITE, national_dex_number_box);
    video_overlay_set.add(COLOR_BLUE, shiny_symbol_box);
    video_overlay_set.add(COLOR_RED, gmax_symbol_box);
    video_overlay_set.add(COLOR_RED, alpha_box);
    video_overlay_set.add(COLOR_DARKGREEN, origin_symbol_box);
    video_overlay_set.add(COLOR_DARK_BLUE, pokemon_box);
    video_overlay_set.add(COLOR_RED, level_box);
    video_overlay_set.add(COLOR_RED, ot_id_box);
    video_overlay_set.add(COLOR_RED, ot_box);
    video_overlay_set.add(COLOR_RED, nature_box);
    video_overlay_set.add(COLOR_RED, ability_box);
    video_overlay_set.add(COLOR_RED, type_box);
    BoxGenderDetector::make_overlays(video_overlay_set);


    // Wait for the summary screen transition to end
    FrozenImageDetector frozen_image_detector(COLOR_GREEN, { 0.388, 0.238, 0.109, 0.062 }, Milliseconds(80), 20);
    frozen_image_detector.make_overlays(video_overlay_set);
    wait_until(env.console, context, 5s, { frozen_image_detector });

    VideoSnapshot screen = env.console.video().snapshot();

    const int dex_number = OCR::read_number_waterfill(env.console, extract_box_reference(screen, national_dex_number_box), 0xff808080, 0xffffffff);
    if (dex_number <= 0 || dex_number > static_cast<int>(NATIONAL_DEX_SLUGS().size())) {
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "BoxSorter Check Summary: Unable to read a correct dex number, found: " + std::to_string(dex_number),
            env.console
        );
    }
    cur_pokemon_info.dex_number = (uint16_t)dex_number;
    cur_pokemon_info.name_slug = NATIONAL_DEX_SLUGS()[dex_number - 1];

    const int shiny_stddev_value = (int)image_stddev(extract_box_reference(screen, shiny_symbol_box)).sum();
    const bool is_shiny = shiny_stddev_value > 30;
    cur_pokemon_info.shiny = is_shiny;
    env.console.log("Shiny detection stddev:" + std::to_string(shiny_stddev_value) + " is shiny:" + std::to_string(is_shiny));

    const int gmax_stddev_value = (int)image_stddev(extract_box_reference(screen, gmax_symbol_box)).sum();
    const bool is_gmax = gmax_stddev_value > 30;
    cur_pokemon_info.gmax = is_gmax;
    env.console.log("Gmax detection stddev:" + std::to_string(gmax_stddev_value) + " is gmax:" + std::to_string(is_gmax));

    const int alpha_stddev_value = (int)image_stddev(extract_box_reference(screen, alpha_box)).sum();
    const bool is_alpha = alpha_stddev_value > 40;
    cur_pokemon_info.alpha = is_alpha;
    env.console.log("Alpha detection stddev:" + std::to_string(alpha_stddev_value) + " is alpha:" + std::to_string(is_alpha));

    BallReader ball_reader(env.console);
    cur_pokemon_info.ball_slug = ball_reader.read_ball(screen);

    const StatsHuntGenderFilter gender = BoxGenderDetector::detect(screen);
    env.console.log("Gender: " + gender_to_string(gender), COLOR_GREEN);
    cur_pokemon_info.gender = gender;

    const int ot_id = OCR::read_number_waterfill(env.console, extract_box_reference(screen, ot_id_box), 0xff808080, 0xffffffff);
    if (ot_id < 0 || ot_id > 999'999) {
        dump_image(env.console, ProgramInfo(), "ReadSummary_OT", screen);
    }
    cur_pokemon_info.ot_id = ot_id;

    auto [primaryType, secondaryType] = read_pokemon_types(screen, type_box, PokemonTypeGeneration::GEN9);

    cur_pokemon_info.primaryType = primaryType;
    cur_pokemon_info.secondaryType = secondaryType;

    env.add_overlay_log(create_overlay_info(cur_pokemon_info));
    video_overlay_set.clear();

    // NOTE edit when adding new struct members (detections go here likely)

    // level_box
    // ot_box
    // nature_box
    // ability_box

    // Press button R to go to next summary screen
    pbf_press_button(context, BUTTON_R, 80ms, 300ms);
    context.wait_for_all_requests();
}

void print_boxes_data(const std::vector<std::optional<CollectedPokemonInfo>>& boxes_data, SingleSwitchProgramEnvironment& env) {
    std::ostringstream ss;
    for (const std::optional<CollectedPokemonInfo>& pokemon : boxes_data) {
        ss << pokemon << "\n";
    }
    env.console.log(ss.str());
}

void exit_menus(SingleSwitchProgramEnvironment& env, ProControllerContext& context, std::chrono::milliseconds video_delay) {
    VideoSnapshot screen = env.console.video().snapshot();
    VideoOverlaySet video_overlay_set(env.console);

    ImageFloatBox select_check(0.495, 0.0045, 0.01, 0.005); // square color to check which mode is active
    FloatPixel image_value = image_stats(extract_box_reference(screen, select_check)).average;

    env.console.log("Color detected from the select square: " + image_value.to_string());

    //if the correct color is not detected, getting out of every possible menu to make sure the program work no matter where you start it in your pokemon home
    video_overlay_set.add(COLOR_BLUE, select_check);
    if (image_value.r <= image_value.g + image_value.b) {
        for (int var = 0; var < 5; ++var) {
            pbf_press_button(context, BUTTON_B, 80ms, video_delay + 80ms);
        }
        context.wait_for_all_requests();
        context.wait_for(std::chrono::milliseconds(video_delay));
        screen = env.console.video().snapshot();
        image_value = image_stats(extract_box_reference(screen, select_check)).average;
        env.console.log("Color detected from the select square: " + image_value.to_string());
        if (image_value.r <= image_value.g + image_value.b) {
            for (int i = 0; i < 2; ++i) {
                pbf_press_button(context, BUTTON_ZR, 80ms, video_delay + 240ms); //additional delay because this animation is slower than the rest
                context.wait_for_all_requests();
                screen = env.console.video().snapshot();
                image_value = image_stats(extract_box_reference(screen, select_check)).average;
                env.console.log("Color detected from the select square: " + image_value.to_string());
                if (image_value.r > image_value.g + image_value.b) {
                    break;
                }
                else if (i == 1) {
                    dump_image(env.console, ProgramInfo(), "SelectSquare", screen);
                    env.console.log("ERROR: Could not find correct color mode please check color logs and timings\n", COLOR_RED);
                    return;
                }
            }
        }
    }

    video_overlay_set.clear();
}

}
}
}
