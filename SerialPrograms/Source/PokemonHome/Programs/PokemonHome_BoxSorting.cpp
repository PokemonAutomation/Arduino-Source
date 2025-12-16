/*  Home Box Sorting
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

/* TODO ideas
break into smaller functions
read pokemon name and store the slug (easier to detect missread than reading a number)
Optimise the swapping algo
Add enum for ball ? Also, BDSP is reading from swsh data. Worth refactoring ?

ideas for more checks :
ability
nature
type
original game
OT
moves
stats
level
surname
language
"stamps"
*/

#include <array>
#include <map>
#include <optional>
#include <sstream>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/FrozenImageDetector.h"
#include "CommonTools/OCR/OCR_NumberReader.h"
#include "CommonTools/StartupChecks/StartProgramChecks.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Resources/Pokemon_PokemonNames.h"
#include "Pokemon/Resources/Pokemon_PokemonSlugs.h"
#include "Pokemon/Pokemon_CollectedPokemonInfo.h"
#include "PokemonHome/Inference/PokemonHome_BoxGenderDetector.h"
#include "PokemonHome/Inference/PokemonHome_BallReader.h"
#include "PokemonHome_BoxSorting.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{
using namespace ::PokemonAutomation::Pokemon;



const size_t MAX_BOXES = 200;
const size_t MAX_COLUMNS = 6;
const size_t MAX_ROWS = 5;

BoxSorting_Descriptor::BoxSorting_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonHome:BoxSorter",
        STRING_POKEMON + " Home", "Box Sorter",
        "Programs/PokemonHome/BoxSorter.html",
        "Order boxes of " + STRING_POKEMON + ".",
        ProgramControllerClass::StandardController_RequiresPrecision,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        {}
    )
{}
struct BoxSorting_Descriptor::Stats : public StatsTracker{
    Stats()
        : pkmn(m_stats["Pokemon"])
        , empty(m_stats["Empty Slots"])
        , compare(m_stats["Compares"])
        , swaps(m_stats["Swaps"])
    {
        m_display_order.emplace_back(Stat("Pokemon"));
        m_display_order.emplace_back(Stat("Empty Slots"));
        m_display_order.emplace_back(Stat("Compares"));
        m_display_order.emplace_back(Stat("Swaps"));
    }
    std::atomic<uint64_t>& pkmn;
    std::atomic<uint64_t>& empty;
    std::atomic<uint64_t>& compare;
    std::atomic<uint64_t>& swaps;
};
std::unique_ptr<StatsTracker> BoxSorting_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

BoxSorting::BoxSorting()
    : BOX_NUMBER(
        "<b>Number of Boxes to Sort:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        1, 1, MAX_BOXES
        )
    , VIDEO_DELAY(
          "<b>Capture Card Delay:</b>",
          LockMode::LOCK_WHILE_RUNNING,
          50
          )
    , GAME_DELAY(
          "<b>" + STRING_POKEMON + " Home App Delay:</b>",
          LockMode::LOCK_WHILE_RUNNING,
          30
          )
    , SORT_TABLE(
          "<b>Sort Order Rules:</b><br>Sort order rules will be applied top to bottom."
          )
    , OUTPUT_FILE(
          false,
          "<b>Output File:</b><br>JSON file basename to catalogue box info found by the program.",
          LockMode::LOCK_WHILE_RUNNING,
          "box_order",
          "box_order"
          )
    , DRY_RUN(
          "<b>Dry Run:</b><br>Catalogue and make sorting plan without execution. Check output at <output_file>.json and <output_file>-sorted.json)",
          LockMode::LOCK_WHILE_RUNNING,
          false
          )
    , NOTIFICATIONS({
          &NOTIFICATION_PROGRAM_FINISH
      })
{
    PA_ADD_OPTION(BOX_NUMBER); //number of box to check and sort
    PA_ADD_OPTION(VIDEO_DELAY); //delay for every input that need video feedback, user will be able to modify this to enhance capture card delay compatibility
    PA_ADD_OPTION(GAME_DELAY);  //delay for non video feedback, this way I can go as fast as pokemon home can handle movement when needed
    PA_ADD_OPTION(SORT_TABLE);
    PA_ADD_OPTION(OUTPUT_FILE);
    PA_ADD_OPTION(DRY_RUN);
    PA_ADD_OPTION(NOTIFICATIONS);
}



struct Cursor{
    size_t box;
    size_t row;
    size_t column;
};

std::ostream& operator<<(std::ostream& os, const Cursor& cursor){
    os << "(" << cursor.box << "/" << cursor.row << "/" << cursor.column << ")";
    return os;
}

Cursor get_cursor(size_t index){
    Cursor ret;

    ret.column = index % MAX_COLUMNS;
    index = index / MAX_COLUMNS;

    ret.row = index % MAX_ROWS;
    index = index / MAX_ROWS;

    ret.box = index;
    return ret;
}

size_t get_index(size_t box, size_t row, size_t column){
    return box * MAX_ROWS * MAX_COLUMNS + row * MAX_COLUMNS + column;
}



// CollectedPokemonInfo struct and related functions have been moved to Pokemon/Pokemon_CollectedPokemonInfo.h/cpp

// Move the red cursor to the first slot of the box
// If the cursor is not add the first slot, move the cursor to the left and up one row at a time until it is at the first slot. 
bool go_to_first_slot(SingleSwitchProgramEnvironment& env, ProControllerContext& context, uint16_t VIDEO_DELAY){
    ImageFloatBox first_slot_cursor_box(0.07, 0.15, 0.01, 0.01); //cursor position of the first slot of the box
    VideoSnapshot screen = env.console.video().snapshot();
    FloatPixel first_slot_cursor_color = image_stats(extract_box_reference(screen, first_slot_cursor_box)).average;
    env.console.log("Cursor color detection: " + first_slot_cursor_color.to_string());
    VideoOverlaySet BoxRender(env.console);
    BoxRender.add(COLOR_BLUE, first_slot_cursor_box);

    // If the cursor is not at the first slot
    if(first_slot_cursor_color.r <= first_slot_cursor_color.g + first_slot_cursor_color.b){
        bool cursor_found = false;
        for (uint8_t rows = 0; rows < 7; rows++){
            for (uint8_t column = 0; column < 5; column++){
                pbf_press_dpad(context, DPAD_LEFT, 10, VIDEO_DELAY);
                context.wait_for_all_requests();
                screen = env.console.video().snapshot();
                first_slot_cursor_color = image_stats(extract_box_reference(screen, first_slot_cursor_box)).average;
                env.console.log("Cursor color detection: " + first_slot_cursor_color.to_string());

                if(first_slot_cursor_color.r > first_slot_cursor_color.g + first_slot_cursor_color.b){
                    cursor_found = true;
                    break;
                }
            }
            if(!cursor_found){
                pbf_press_dpad(context, DPAD_UP, 10, VIDEO_DELAY);
                context.wait_for_all_requests();
                screen = env.console.video().snapshot();
                first_slot_cursor_color = image_stats(extract_box_reference(screen, first_slot_cursor_box)).average;
                env.console.log("Cursor color detection: " + first_slot_cursor_color.to_string());

                if(first_slot_cursor_color.r > first_slot_cursor_color.g + first_slot_cursor_color.b){
                    cursor_found = true;
                    break;
                }
            }else{
                break;
            }
        }
        if(!cursor_found){
            return false;
        }
    }
    BoxRender.clear();
    return true;
}

//Move the cursor to the given coordinates, knowing current pos via the cursor struct
[[nodiscard]] Cursor move_cursor_to(SingleSwitchProgramEnvironment& env, ProControllerContext& context, const Cursor& cur_cursor, const Cursor& dest_cursor, uint16_t GAME_DELAY){

    std::ostringstream ss;
    ss << "Moving cursor from " << cur_cursor << " to " << dest_cursor;
    env.console.log(ss.str());

    // TODO: shortest path movement though pages, boxes
    for (size_t i = cur_cursor.box; i < dest_cursor.box; ++i){
        pbf_press_button(context, BUTTON_R, 10, GAME_DELAY+30);
    }
    for (size_t i = dest_cursor.box; i < cur_cursor.box; ++i){
        pbf_press_button(context, BUTTON_L, 10, GAME_DELAY+30);
    }


    // direct nav up or down through rows
    if (!(cur_cursor.row == 0 && dest_cursor.row == 4) && !(dest_cursor.row == 0 && cur_cursor.row == 4)){
        for (size_t i = cur_cursor.row; i < dest_cursor.row; ++i){
            pbf_press_dpad(context, DPAD_DOWN, 10, GAME_DELAY);
        }
        for (size_t i = dest_cursor.row; i < cur_cursor.row; ++i){
            pbf_press_dpad(context, DPAD_UP, 10, GAME_DELAY);
        }
    }else{ // wrap around is faster to move between first or last row
        if (cur_cursor.row == 0 && dest_cursor.row == 4){
            for (size_t i = 0; i <= 2; ++i){
                pbf_press_dpad(context, DPAD_UP, 10, GAME_DELAY);
            }
        }else{
            for (size_t i = 0; i <= 2; ++i){
                pbf_press_dpad(context, DPAD_DOWN, 10, GAME_DELAY);
            }
        }
    }

    // direct nav forward or backward through columns
    if ((dest_cursor.column > cur_cursor.column && dest_cursor.column - cur_cursor.column <= 3) || (cur_cursor.column > dest_cursor.column && cur_cursor.column - dest_cursor.column <= 3)){
        for (size_t i = cur_cursor.column; i < dest_cursor.column; ++i){
            pbf_press_dpad(context, DPAD_RIGHT, 10, GAME_DELAY);
        }
        for (size_t i = dest_cursor.column; i < cur_cursor.column; ++i){
            pbf_press_dpad(context, DPAD_LEFT, 10, GAME_DELAY);
        }
    }else{ // wrap around is faster if direct movement is more than 3 away
        if (dest_cursor.column > cur_cursor.column){
            for (size_t i = 0; i < MAX_COLUMNS - (dest_cursor.column - cur_cursor.column); ++i){
                pbf_press_dpad(context, DPAD_LEFT, 10, GAME_DELAY);
            }
        }
        if (cur_cursor.column > dest_cursor.column){
            for (size_t i = 0; i < MAX_COLUMNS - (cur_cursor.column - dest_cursor.column); ++i){
                pbf_press_dpad(context, DPAD_RIGHT, 10, GAME_DELAY);
            }
        }
    }

    context.wait_for_all_requests();
    return dest_cursor;
}

void print_boxes_data(const std::vector<std::optional<CollectedPokemonInfo>>& boxes_data, SingleSwitchProgramEnvironment& env){
    std::ostringstream ss;
    for (const std::optional<CollectedPokemonInfo>& pokemon : boxes_data){
        ss << pokemon << "\n";
    }
    env.console.log(ss.str());
}

void output_boxes_data_json(const std::vector<std::optional<CollectedPokemonInfo>>& boxes_data, const std::string& json_path_basename){
    JsonArray pokemon_data;
    for (size_t poke_nb = 0; poke_nb < boxes_data.size(); poke_nb++){
        Cursor cursor = get_cursor(poke_nb);
        JsonObject pokemon;
        pokemon["index"] = poke_nb;
        pokemon["box"] = cursor.box;
        pokemon["row"] =  cursor.row;
        pokemon["column"] =  cursor.column;
        const auto& current_pokemon = boxes_data[poke_nb];
        if (current_pokemon != std::nullopt){
            // NOTE edit when adding new struct members
            pokemon["national_dex_number"] = current_pokemon->national_dex_number;
            pokemon["shiny"] = current_pokemon->shiny;
            pokemon["gmax"] = current_pokemon->gmax;
            pokemon["alpha"] = current_pokemon->alpha;
            pokemon["ball_slug"] = current_pokemon->ball_slug;
            pokemon["gender"] = gender_to_string(current_pokemon->gender);
            pokemon["ot_id"] = current_pokemon->ot_id;
        }
        pokemon_data.push_back(std::move(pokemon));
    }
    pokemon_data.dump(json_path_basename + ".json");
}

void do_sort(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    std::vector<std::optional<CollectedPokemonInfo>> boxes_data,
    std::vector<std::optional<CollectedPokemonInfo>> boxes_sorted,
    BoxSorting_Descriptor::Stats& stats,
    Cursor& cur_cursor,
    uint16_t GAME_DELAY
){
    env.log("Start sorting...");
    env.add_overlay_log("Start Sorting...");

    std::ostringstream ss;
    // this need to be separated into functions when I will redo the whole thing but I just wanted it to work

    // going thru the sorted list one by one and for each one go through the current pokemon layout to find the closest possible match to fill the slot
    for (size_t poke_nb_s = 0; poke_nb_s < boxes_sorted.size(); poke_nb_s++){
        if (boxes_sorted[poke_nb_s] == std::nullopt){ // we've hit the end of the sorted list.
            break;
        }
        for (size_t poke_nb = poke_nb_s; poke_nb < boxes_data.size(); poke_nb++){
            Cursor cursor_s = get_cursor(poke_nb_s);
            Cursor cursor = get_cursor(poke_nb);

            // ss << "Comparing " << boxes_data[poke_nb] << " at " << cursor << " to " << boxes_sorted[poke_nb_s] << " at " << cursor_s;
            // env.console.log(ss.str());
            // ss.str("");

            //check for a match and also check if the pokemon is not already in the slot
            stats.compare++;
            env.update_stats();
            if(boxes_sorted[poke_nb_s] == boxes_data[poke_nb] && poke_nb_s == poke_nb){ // Same spot no need to move.
                break;
            }
            if(boxes_sorted[poke_nb_s] == boxes_data[poke_nb]){
                ss << "Swapping " << boxes_data[poke_nb] << " at " << cursor << " and " << boxes_sorted[poke_nb_s] << " at " << cursor_s;
                env.console.log(ss.str());
                ss.str("");

                //moving cursor to the pokemon to pick it up
                cur_cursor = move_cursor_to(env, context, cur_cursor, cursor, GAME_DELAY);
                pbf_press_button(context, BUTTON_Y, 10, GAME_DELAY+30);

                //moving to destination to place it or swap it
                cur_cursor = move_cursor_to(env, context, cur_cursor, cursor_s, GAME_DELAY);
                pbf_press_button(context, BUTTON_Y, 10, GAME_DELAY+30);

                context.wait_for_all_requests();

                std::swap(boxes_data[poke_nb_s], boxes_data[poke_nb]);
                stats.swaps++;
                env.update_stats();

                break;
            }
        }
    }
}

void BoxSorting::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    StartProgramChecks::check_performance_class_wired_or_wireless(context);

    std::vector<SortingRule> sort_preferences = SORT_TABLE.preferences();
    if (sort_preferences.empty()){
        throw UserSetupError(env.console, "At least one sorting method selection needs to be made!");
    }

    BoxSorting_Descriptor::Stats& stats = env.current_stats< BoxSorting_Descriptor::Stats>();

    ImageFloatBox select_check(0.495, 0.0045, 0.01, 0.005); // square color to check which mode is active
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


    // vector that will store data for each slot
    std::vector<std::optional<CollectedPokemonInfo>> boxes_data;

    Cursor cur_cursor{static_cast<uint16_t>(BOX_NUMBER-1), 0, 0};

    VideoSnapshot screen = env.console.video().snapshot();

    VideoOverlaySet box_render(env.console);

    std::ostringstream ss;

    FloatPixel image_value = image_stats(extract_box_reference(screen, select_check)).average;

    env.console.log("Color detected from the select square: " + image_value.to_string());

    //if the correct color is not detected, getting out of every possible menu to make sure the program work no matter where you start it in your pokemon home
    box_render.add(COLOR_BLUE, select_check);
    if(image_value.r <= image_value.g + image_value.b){
        for (int var = 0; var < 5; ++var){
            pbf_press_button(context, BUTTON_B, 10, GAME_DELAY+10);
        }
        context.wait_for_all_requests();
        context.wait_for(std::chrono::milliseconds(VIDEO_DELAY));
        screen = env.console.video().snapshot();
        image_value = image_stats(extract_box_reference(screen, select_check)).average;
        env.console.log("Color detected from the select square: " + image_value.to_string());
        if(image_value.r <= image_value.g + image_value.b){
            for (int i = 0; i < 2; ++i){
                pbf_press_button(context, BUTTON_ZR, 10, VIDEO_DELAY+30); //additional delay because this animation is slower than the rest
                context.wait_for_all_requests();
                screen = env.console.video().snapshot();
                image_value = image_stats(extract_box_reference(screen, select_check)).average;
                env.console.log("Color detected from the select square: " + image_value.to_string());
                if(image_value.r > image_value.g + image_value.b){
                    break;
                }else if(i==1){
                    dump_image(env.console, ProgramInfo(), "SelectSquare", screen);
                    env.console.log("ERROR: Could not find correct color mode please check color logs and timings\n", COLOR_RED);
                    return;
                }
            }
        }
    }

    box_render.clear();

    Cursor dest_cursor;
    Cursor nav_cursor = {0, 0, 0};

    //cycle through each box
    for (size_t box_idx = 0; box_idx < BOX_NUMBER; box_idx++){
        if(box_idx != 0){
            // Press button R to move to next box
            pbf_press_button(context, BUTTON_R, 10, VIDEO_DELAY+100);
        }else{
            // Moving the cursor until it goes to the first slot
            if(!go_to_first_slot(env, context, VIDEO_DELAY)){
                env.console.log("ERROR: Could not move cursor to the first slot, please consider adjusting delay\n", COLOR_RED);
                return;
            }
        }

        context.wait_for_all_requests();
        const std::string log_msg = "Checking box " + std::to_string(box_idx+1) + "/" + std::to_string(BOX_NUMBER);
        env.log(log_msg);
        env.console.overlay().add_log(log_msg);

        screen = env.console.video().snapshot();

        // Box grid to find empty slots (red boxes) and fill boxes_data with value to check or not for pokemon dex number

        ss << "\n";

        std::array<size_t, 2> first_poke_slot = {0, 0};
        bool find_first_poke = false;

        int num_empty_slots = 0;
        for (size_t row = 0; row < MAX_ROWS; row++){
            for (size_t column = 0; column < MAX_COLUMNS; column++){
                ImageFloatBox slot_box(0.06 + (0.072 * column), 0.2 + (0.1035 * row), 0.03, 0.057);
                int current_box_value = (int)image_stddev(extract_box_reference(screen, slot_box)).sum();

                ss << current_box_value;

                //checking color to know if a pokemon is on the slot or not
                if(current_box_value < 10){
                    box_render.add(COLOR_RED, slot_box);
                    stats.empty++;
                    num_empty_slots++;
                    env.update_stats();
                    boxes_data.push_back(std::nullopt); //empty optional to make sorting easier later
                    ss << "\u274c " ;    //  "X"
                }else{
                    if(find_first_poke == false){
                        first_poke_slot = {column, row};
                        find_first_poke = true;
                    }
                    box_render.add(COLOR_GREEN, slot_box);
                    stats.pkmn++;
                    env.update_stats();
                    boxes_data.push_back(
                        CollectedPokemonInfo{
                            .preferences = &sort_preferences
                        }
                        ); //default initialised pokemon to know there is a pokemon here that needs a value
                    ss << "\u2705 " ;    //  checkbox
                }
            }
            ss << "\n";
        }
        env.console.log(ss.str());
        env.add_overlay_log("Empty: " + std::to_string(num_empty_slots) + "/30");
        ss.str("");

        // moving cursor to the first pokemon slot
        dest_cursor = {0, first_poke_slot[1], first_poke_slot[0]};
        nav_cursor = move_cursor_to(env, context, nav_cursor, dest_cursor, GAME_DELAY);

        //enter the summary screen to read pokemon info
        if (find_first_poke){
            env.add_overlay_log("Checking Summary...");

            pbf_press_button(context, BUTTON_A, 10, GAME_DELAY);
            context.wait_for_all_requests();
            box_render.clear();
            pbf_press_dpad(context, DPAD_DOWN, 10, GAME_DELAY);
            pbf_press_button(context, BUTTON_A, 10, VIDEO_DELAY+150);
            context.wait_for_all_requests();

            box_render.add(COLOR_WHITE, national_dex_number_box);
            box_render.add(COLOR_BLUE, shiny_symbol_box);
            box_render.add(COLOR_RED, gmax_symbol_box);
            box_render.add(COLOR_RED, alpha_box);
            box_render.add(COLOR_DARKGREEN, origin_symbol_box);
            box_render.add(COLOR_DARK_BLUE, pokemon_box);
            box_render.add(COLOR_RED, level_box);
            box_render.add(COLOR_RED, ot_id_box);
            box_render.add(COLOR_RED, ot_box);
            box_render.add(COLOR_RED, nature_box);
            box_render.add(COLOR_RED, ability_box);

            // cycle through each summary of the current box and fill pokemon information
            for (size_t row = 0; row < MAX_ROWS; row++){
                for (size_t column = 0; column < MAX_COLUMNS; column++){
                    const size_t global_idx = get_index(box_idx, row, column);
                    if (!boxes_data[global_idx].has_value()){
                        continue;
                    }

                    // Wait for the summary screen transition to end
                    FrozenImageDetector frozen_image_detector(COLOR_GREEN, {0.388, 0.238, 0.109, 0.062}, Milliseconds(80), 20);
                    frozen_image_detector.make_overlays(box_render);
                    wait_until(env.console, context, 5s, {frozen_image_detector});

                    auto& cur_pokemon_info = boxes_data[global_idx];
                    screen = env.console.video().snapshot();

                    const int national_dex_number = OCR::read_number_waterfill(env.console, extract_box_reference(screen, national_dex_number_box), 0xff808080, 0xffffffff);
                    if (national_dex_number <= 0 || national_dex_number > static_cast<int>(NATIONAL_DEX_SLUGS().size())) {
                        OperationFailedException::fire(
                            ErrorReport::SEND_ERROR_REPORT,
                            "BoxSorting Check Summary: Unable to read a correct dex number, found: " + std::to_string(national_dex_number),
                            env.console
                        );
                    }
                    cur_pokemon_info->national_dex_number = (uint16_t)national_dex_number;

                    const int shiny_stddev_value = (int)image_stddev(extract_box_reference(screen, shiny_symbol_box)).sum();
                    const bool is_shiny = shiny_stddev_value > 30;
                    cur_pokemon_info->shiny = is_shiny;
                    env.console.log("Shiny detection stddev:" + std::to_string(shiny_stddev_value) + " is shiny:" + std::to_string(is_shiny));

                    const int gmax_stddev_value = (int)image_stddev(extract_box_reference(screen, gmax_symbol_box)).sum();
                    const bool is_gmax = gmax_stddev_value > 30;
                    cur_pokemon_info->gmax = is_gmax;
                    env.console.log("Gmax detection stddev:" + std::to_string(gmax_stddev_value) + " is gmax:" + std::to_string(is_gmax));

                    const int alpha_stddev_value = (int)image_stddev(extract_box_reference(screen, alpha_box)).sum();
                    const bool is_alpha = alpha_stddev_value > 40;
                    cur_pokemon_info->alpha = is_alpha;
                    env.console.log("Alpha detection stddev:" + std::to_string(alpha_stddev_value) + " is alpha:" + std::to_string(is_alpha));

                    BallReader ball_reader(env.console);
                    cur_pokemon_info->ball_slug = ball_reader.read_ball(screen);

                    BoxGenderDetector::make_overlays(box_render);
                    const StatsHuntGenderFilter gender = BoxGenderDetector::detect(screen);
                    env.console.log("Gender: " + gender_to_string(gender), COLOR_GREEN);
                    cur_pokemon_info->gender = gender;

                    const int ot_id = OCR::read_number_waterfill(env.console, extract_box_reference(screen, ot_id_box), 0xff808080, 0xffffffff);
                    if (ot_id < 0 || ot_id > 999'999) {
                        dump_image(env.console, ProgramInfo(), "ReadSummary_OT", screen);
                    }
                    cur_pokemon_info->ot_id = ot_id;
                    
                    env.add_overlay_log(create_overlay_info(*cur_pokemon_info));

                    // NOTE edit when adding new struct members (detections go here likely)

                    // level_box
                    // ot_box
                    // nature_box
                    // ability_box

                    // Press button R to go to next summary screen
                    pbf_press_button(context, BUTTON_R, 10, VIDEO_DELAY+15);
                    context.wait_for_all_requests();
                }
            }

            box_render.clear();

            ss << std::endl;

            // print box information
            for (size_t row = 0; row < MAX_ROWS; row++){
                for (size_t column = 0; column < MAX_COLUMNS; column++){
                    ss << boxes_data[get_index(box_idx, row, column)] << " ";
                }
                ss << std::endl;
            }

            env.console.log(ss.str());
            ss.str("");

            //get out of summary with a lot of delay because it's slow for some reasons
            pbf_press_button(context, BUTTON_B, 10, VIDEO_DELAY+250);
            box_render.clear();
            context.wait_for_all_requests();
        }

        box_render.clear();

        dest_cursor = {0, 0, 0};
        nav_cursor = move_cursor_to(env, context, nav_cursor, dest_cursor, GAME_DELAY);
        context.wait_for_all_requests();
    } // end if (find_first_poke){

    // copy boxes data to sort
    std::vector<std::optional<CollectedPokemonInfo>> boxes_sorted = boxes_data;

    // sorting copy of boxes_data
    std::sort(boxes_sorted.begin(), boxes_sorted.end());

    env.console.log("Current boxes data :");
    print_boxes_data(boxes_data, env);
    const std::string json_path_basename = OUTPUT_FILE;
    output_boxes_data_json(boxes_data, json_path_basename);

    env.console.log("Sorted boxes data :");
    print_boxes_data(boxes_sorted, env);
    const std::string sorted_path = json_path_basename + "-sorted";
    output_boxes_data_json(boxes_sorted, sorted_path);

    if (!DRY_RUN){
        do_sort(env, context, boxes_data, boxes_sorted, stats, cur_cursor, GAME_DELAY);
    }

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);

}

}
}
}

