/*  Home Box Sorting
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

/* TODO ideas
break into smaller functions
use "flat" vector instead of 3D (it's easier to manipulate and avoid "translation")
read pokemon name and store the slug (easier to detect missread than reading a number)
optimize the search for fitting pokemon by starting from box_nb, column and line instead of 0
Optimise the swapping algo
Add enum for ball ? Also, BDSP is reading from swsh data. Worth refactoring ?
Find a way to let the user customize the sorting algo based on their preference

ideas for more checks :
gender
ability
nature
type
original game
OT
ID OT
moves
stats
level
surname
language
"stamps"
*/

#include <map>
#include <optional>
#include <sstream>
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageFilter.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/OCR/OCR_NumberReader.h"
#include "CommonFramework/OCR/OCR_TextMatcher.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonHome/Inference/PokemonHome_BallReader.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_GameEntry.h"
#include "PokemonSwSh/Programs/ReleaseHelpers.h"
#include "PokemonHome_BoxSorting.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{
    using namespace Pokemon;

namespace{
}

BoxSorting_Descriptor::BoxSorting_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonHome:BoxSorter",
        STRING_POKEMON + " Home", "Box Sorter",
        "ComputerControl/blob/master/Wiki/Programs/PokemonHome/BoxSorter.md",
        "Order boxes of " + STRING_POKEMON + ".",
        FeedbackType::REQUIRED, true, false,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}

BoxSorting::BoxSorting()
        : BOX_NUMBER(
            "<b>Number of boxes to order:</b>",
            1
        ),
        VIDEO_DELAY(
            "<b>Delay of your capture card (you probably have to increase this):</b>",
            20
        ),
        GAME_DELAY(
            "<b>Delay of your Pokemon Home app (default value should be fine):</b>",
            10
        ),
        NOTIFICATIONS({
            &NOTIFICATION_PROGRAM_FINISH
        })
    {
        PA_ADD_OPTION(BOX_NUMBER); //number of box to check and sort
        PA_ADD_OPTION(VIDEO_DELAY); //delay for every input that need video feedback, user will be able to modify this to enhance capture card delay compatibility
        PA_ADD_OPTION(GAME_DELAY);  //delay for non video feedback, this way I can go as fast as pokemon home can handle movement when needed
        PA_ADD_OPTION(NOTIFICATIONS);
    }

bool go_to_first_slot(SingleSwitchProgramEnvironment& env, BotBaseContext& context, uint16_t VIDEO_DELAY){

    ImageFloatBox cursor_check(0.07, 0.15, 0.01, 0.01); //cursor position of the first slot of the box
    std::shared_ptr<const ImageRGB32> screen = env.console.video().snapshot();
    FloatPixel image_value = image_stats(extract_box_reference(*screen, cursor_check)).average;
    env.console.log("Cursor color detection: " + image_value.to_string());
    VideoOverlaySet BoxRender(env.console);

    BoxRender.add(COLOR_BLUE, cursor_check);
    if(image_value.r <= image_value.g + image_value.b){

        bool cursor_found = false;

        for (uint8_t lines = 0; lines < 7; lines++) {
            for (uint8_t column = 0; column < 5; column++) {
                pbf_press_dpad(context, DPAD_LEFT, 10, VIDEO_DELAY);
                context.wait_for_all_requests();
                screen = env.console.video().snapshot();
                image_value = image_stats(extract_box_reference(*screen, cursor_check)).average;
                env.console.log("Cursor color detection: " + image_value.to_string());

                if(image_value.r > image_value.g + image_value.b){
                    cursor_found = true;
                    break;
                }
            }
            if(!cursor_found){
                pbf_press_dpad(context, DPAD_UP, 10, VIDEO_DELAY);
                context.wait_for_all_requests();
                screen = env.console.video().snapshot();
                image_value = image_stats(extract_box_reference(*screen, cursor_check)).average;
                env.console.log("Cursor color detection: " + image_value.to_string());

                if(image_value.r > image_value.g + image_value.b){
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

struct Cursor{
  uint16_t box;
  uint16_t column;
  uint16_t line;
};

struct Pokemon{
    // When adding any new member here, do not forget to modify the three operators below
    uint16_t dex_number = 0;
    bool shiny = false;
    bool gmax = false;
    std::string ball_slug = "";
};

bool operator==(const Pokemon& lhs, const Pokemon& rhs){
    return lhs.dex_number == rhs.dex_number &&
        lhs.shiny == rhs.shiny &&
        lhs.gmax == rhs.gmax &&
        lhs.ball_slug == rhs.ball_slug;
}

bool operator<(const std::optional<Pokemon>& lhs, const std::optional<Pokemon>& rhs){
    if (!lhs.has_value()){
        return false;
    }
    if (!rhs.has_value()){
        return true;
    }
    // For example, we could imagine that people want to sort first by ball type, then by dex number.
    // if (lhs->ball_slug < rhs->ball_slug){
    //     return true;
    // }
    // if (lhs->ball_slug > rhs->ball_slug){
    //     return false;
    // }
    // if (lhs->shiny != rhs->shiny){
    //     return lhs->shiny;
    // }
    // if (lhs->gmax != rhs->gmax){
    //     return lhs->gmax;
    // }
    return lhs->dex_number < rhs->dex_number;
}

std::ostream& operator<<(std::ostream& os, const std::optional<Pokemon>& pokemon)
{
    if (pokemon.has_value()){
        os << "(";
        os << "dex_number:" << pokemon->dex_number << " ";
        os << "shiny:" << pokemon->shiny << " ";
        os << "gmax:" << pokemon->gmax << " ";
        os << "ball_slug: " << pokemon->ball_slug;
        os << ")";
    }
    else{
        os << "(empty)";
    }
    return os;
}

//Move the cursor to the given coordinates, knowing current pos via the cursor struct
void move_cursor_to(SingleSwitchProgramEnvironment& env, BotBaseContext& context, uint16_t box_number_to, uint16_t column_to, uint16_t line_to, Cursor* cur_cursor, uint16_t GAME_DELAY){

    std::ostringstream ss;
    ss << "The program think the cursor is at: " << cur_cursor->box << " " << cur_cursor->column << " " << cur_cursor->line;
    env.console.log(ss.str());
    ss.str("");

    if(box_number_to > cur_cursor->box){
        for (int i = 0; i < box_number_to-cur_cursor->box; ++i) {
            pbf_press_button(context, BUTTON_R, 10, GAME_DELAY+30);
        }
    }else{
        for (int i = 0; i < cur_cursor->box-box_number_to; ++i) {
            pbf_press_button(context, BUTTON_L, 10, GAME_DELAY+30);
        }
    }

    if(column_to > cur_cursor->column){
        for (int i = 0; i < column_to-cur_cursor->column; ++i) {
            pbf_press_dpad(context, DPAD_DOWN, 1, GAME_DELAY);
        }
    }else{
        for (int i = 0; i < cur_cursor->column-column_to; ++i) {
            pbf_press_dpad(context, DPAD_UP, 1, GAME_DELAY);
        }
    }

    if(line_to > cur_cursor->line){
        for (int i = 0; i < line_to-cur_cursor->line; ++i) {
            pbf_press_dpad(context, DPAD_RIGHT, 1, GAME_DELAY);
        }
    }else{
        for (int i = 0; i < cur_cursor->line-line_to; ++i) {
            pbf_press_dpad(context, DPAD_LEFT, 1, GAME_DELAY);
        }
    }

    cur_cursor->box = box_number_to;
    cur_cursor->column = column_to;
    cur_cursor->line = line_to;

    context.wait_for_all_requests();
}

void BoxSorting::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){

    ImageFloatBox select_check(0.495, 0.0045, 0.01, 0.005); // square color to check which mode is active
    ImageFloatBox dex_number_box(0.44, 0.245, 0.04, 0.04); //pokemon national dex number pos
    ImageFloatBox shiny_symbol_box(0.702, 0.09, 0.04, 0.06); // shiny symbol pos
    ImageFloatBox gmax_symbol_box(0.463, 0.09, 0.04, 0.06); // gmax symbol pos

    //3 dimensions vector that will store data for each slot
    std::vector<std::vector<std::vector<std::optional<Pokemon>>>> box_data;

    Cursor cur_cursor{static_cast<uint16_t>(BOX_NUMBER-1), 0, 0};

    std::shared_ptr<const ImageRGB32> screen = env.console.video().snapshot();

    VideoOverlaySet box_render(env.console);

    std::ostringstream ss;

    FloatPixel image_value = image_stats(extract_box_reference(*screen, select_check)).average;

    env.console.log("Color detected from the select square: " + image_value.to_string());

    //if the correct color is not detected, getting out of every possible menu to make sure the program work no matter where you start it in your pokemon home
    box_render.add(COLOR_BLUE, select_check);
    if(image_value.r <= image_value.g + image_value.b){
        for (int var = 0; var < 5; ++var) {
            pbf_press_button(context, BUTTON_B, 10, GAME_DELAY+10);
        }
        context.wait_for_all_requests();
        context.wait_for(std::chrono::milliseconds(VIDEO_DELAY));
        screen = env.console.video().snapshot();
        image_value = image_stats(extract_box_reference(*screen, select_check)).average;
        env.console.log("Color detected from the select square: " + image_value.to_string());
        if(image_value.r <= image_value.g + image_value.b){
            for (int i = 0; i < 2; ++i) {
                pbf_press_button(context, BUTTON_ZR, 10, VIDEO_DELAY+20); //additional delay because this animation is slower than the rest
                context.wait_for_all_requests();
                screen = env.console.video().snapshot();
                image_value = image_stats(extract_box_reference(*screen, select_check)).average;
                env.console.log("Color detected from the select square: " + image_value.to_string());
                if(image_value.r > image_value.g + image_value.b){
                    break;
                }else if(i==1){
                    dump_image(env.console, ProgramInfo(), "SelectSquare", *screen);
                    env.console.log("ERROR: Could not find correct color mode please check color logs and timings\n", COLOR_RED);
                    return;
                }
            }
        }
    }

    box_render.clear();

    //cycle through each box
    for (int box_nb = 0; box_nb < BOX_NUMBER; ++box_nb) {

        if(box_nb != 0){
            pbf_press_button(context, BUTTON_R, 10, VIDEO_DELAY+100);
            context.wait_for_all_requests();
        }

        // Moving the cursor until it goes to the first slot
        if(!go_to_first_slot(env, context, VIDEO_DELAY)){
            env.console.log("ERROR: Could not move cursor to the first slot, please consider adjusting delay\n", COLOR_RED);
            return;
        }
        context.wait_for_all_requests();
        screen = env.console.video().snapshot();

        // Box grid to find empty slots (red boxes) and fill box_data with value to check or not for pokemon dex number
        box_data.push_back(std::vector<std::vector<std::optional<Pokemon>>>());

        ss << "\n";

        for (uint8_t i = 0; i < 5; i++) {

            box_data[box_nb].push_back(std::vector<std::optional<Pokemon>>());

            for (uint8_t j = 0; j < 6; j++) {

                ImageFloatBox slot_box(0.06 + (0.072 * j), 0.2 + (0.1035 * i), 0.03, 0.057);
                int current_box_value = image_stddev(extract_box_reference(*screen, slot_box)).sum();

                ss << current_box_value;

                //checking color to know if a pokemon is on the slot or not
                if(current_box_value < 15){
                    box_render.add(COLOR_RED, slot_box);
                    box_data[box_nb][i].push_back(std::nullopt); //empty optional to make sorting easier later
                    ss << "\u274c " ;    //  "X"
                }else{
                    box_render.add(COLOR_GREEN, slot_box);
                    box_data[box_nb][i].push_back(Pokemon{}); //default initialised pokemon to know there is a pokemon here that needs a value
                    ss << "\u2705 " ;    //  checkbox
                }
            }
            ss << "\n";
        }

        env.console.log(ss.str());
        ss.str("");

        bool find_first_poke = false;

        //cycling though each slot of the box in order to find the first pokemon to enter the summary
        for (int column = 0; column < 5; ++column) {
            for (int line = 0; line < 6; ++line) {
                if(box_data[box_nb][column][line].has_value()){
                    find_first_poke = true;
                }
                if(!find_first_poke){
                    pbf_press_dpad(context, DPAD_RIGHT, 10, GAME_DELAY);
                }
            }
            if(!find_first_poke){
                pbf_press_dpad(context, DPAD_DOWN, 10, GAME_DELAY);
            }
        }

        //enter the summary screen
        pbf_press_button(context, BUTTON_A, 10, GAME_DELAY);
        context.wait_for_all_requests();
        box_render.clear();
        pbf_press_dpad(context, DPAD_DOWN, 10, GAME_DELAY);
        pbf_press_button(context, BUTTON_A, 10, VIDEO_DELAY+150);
        context.wait_for_all_requests();

        box_render.add(COLOR_RED, dex_number_box);
        box_render.add(COLOR_BLUE, shiny_symbol_box);
        box_render.add(COLOR_GREEN, gmax_symbol_box);

        //cycle through each summary of the current box and fill pokemon dex number in
        for (int column = 0; column < 5; ++column) {
            for (int line = 0; line < 6; ++line) {

                if(box_data[box_nb][column][line].has_value()){
                    screen = env.console.video().snapshot();
                    ImageRGB32 image = to_blackwhite_rgb32_range(
                        extract_box_reference(*screen, dex_number_box),
                        0xff808080, 0xffffffff, true
                    );

                    int dex_number = OCR::read_number(env.console, image);
                    if (dex_number == -1){
                        dump_image(env.console, ProgramInfo(), "ReadSummary", *screen);
                    }
                    box_data[box_nb][column][line]->dex_number = dex_number;

                    int shiny_stddev_value = image_stddev(extract_box_reference(*screen, shiny_symbol_box)).sum();
                    bool is_shiny = shiny_stddev_value > 30;
                    box_data[box_nb][column][line]->shiny = is_shiny;
                    env.console.log("Shiny detection stddev:" + std::to_string(shiny_stddev_value) + " is shiny:" + std::to_string(is_shiny));

                    int gmax_stddev_value = image_stddev(extract_box_reference(*screen, gmax_symbol_box)).sum();
                    bool is_gmax = gmax_stddev_value > 30;
                    box_data[box_nb][column][line]->gmax = is_gmax;
                    env.console.log("Gmax detection stddev:" + std::to_string(gmax_stddev_value) + " is gmax:" + std::to_string(is_gmax));

                    BallReader ball_reader(env.console);
                    box_data[box_nb][column][line]->ball_slug = ball_reader.read_ball(*screen);

                    pbf_press_button(context, BUTTON_R, 10, VIDEO_DELAY+15);
                    context.wait_for_all_requests();
                }
            }
        }

        box_render.clear();

        ss << std::endl;

        //a useless loop that I should move to the loop above to reduce useless iterations
        for (int column = 0; column < 5; ++column) {
            for (int line = 0; line < 6; ++line) {
                ss << box_data[box_nb][column][line] << " ";
            }
            ss << std::endl;
        }

        env.console.log(ss.str());
        ss.str("");

        //get out of summary with a lot of delay because it's slow for some reasons
        pbf_press_button(context, BUTTON_B, 10, VIDEO_DELAY+200);
        box_render.clear();
        context.wait_for_all_requests();

        go_to_first_slot(env, context, VIDEO_DELAY);
        context.wait_for_all_requests();
    }

    //making a flat list of the pokemons because turns out my original idea of 3d data is turning into a pain to use for sorting
    std::vector<std::optional<Pokemon>> flat_box;

    for (int box_nb = 0; box_nb < int(box_data.size()); ++box_nb) {
        for (int column = 0; column < 5; ++column) {
            for (int line = 0; line < 6; ++line) {
                flat_box.push_back(box_data[box_nb][column][line]);
            }
        }
    }

    //using sort to sort the flat copy of box data
    std::sort(flat_box.begin(), flat_box.end());

    for (const std::optional<Pokemon>& var : flat_box) {
        ss << var << " ";
    }
    env.console.log(ss.str());
    ss.str("");

    //make the flat sorted list 3d box grid again because why not now that I'm here
    std::vector<std::vector<std::vector<std::optional<Pokemon>>>> box_sorted;

    for (int i = 0; i < int(flat_box.size()); ++i) {
        if(i%30==0){
            box_sorted.push_back(std::vector<std::vector<std::optional<Pokemon>>>());
        }
        if(i%6==0){
            box_sorted[i/30].push_back(std::vector<std::optional<Pokemon>>());
        }
        box_sorted[i/30][(i-((i/30)*30))/6].push_back(flat_box[i]);
    }

    ss << std::endl;

    //this need to be separated into functions when I will redo the whole thing but I just wanted it to work

    //going thru the sorted list one by one and for each one go through the current pokemon layout to find the closest possible match to fill the slot
    for (int box_nb = 0; box_nb < int(box_sorted.size()); ++box_nb) {
        for (int column = 0; column < 5; ++column) {
            for (int line = 0; line < 6; ++line) {
                bool found_pkmn = false;
                for (int box_nb_s = 0; box_nb_s < int(box_data.size()); ++box_nb_s) {
                    for (int column_s = 0; column_s < 5; ++column_s) {
                        for (int line_s = 0; line_s < 6; ++line_s) {
                            //check for a match and also check if the pokemon is not already in the slot
                            if(box_sorted[box_nb][column][line] == box_data[box_nb_s][column_s][line_s] && box_data[box_nb_s][column_s][line_s] != box_data[box_nb][column][line]){

                                //moving cursor to the pokemon to pick it up
                                ss << "Moving cursor to " << box_nb_s <<" "<< column_s <<" "<< line_s;
                                env.console.log(ss.str());
                                ss.str("");
                                move_cursor_to(env, context, box_nb_s, column_s, line_s, &cur_cursor, GAME_DELAY);
                                pbf_press_button(context, BUTTON_Y, 10, GAME_DELAY+30);
                                //moving to destination to place it or swap it
                                ss << "Moving "<< box_data[box_nb_s][column_s][line_s] <<" to " << box_nb <<" "<< column <<" "<< line;
                                env.console.log(ss.str());
                                ss.str("");
                                move_cursor_to(env, context, box_nb, column, line, &cur_cursor, GAME_DELAY);
                                pbf_press_button(context, BUTTON_Y, 10, GAME_DELAY+30);
                                context.wait_for_all_requests();
                                box_data[box_nb_s][column_s][line_s] = box_data[box_nb][column][line]; //update the list in case of a pokemon swap
                                box_data[box_nb][column][line] = std::nullopt; //delete pokemon data to make sure it does not match again
                                found_pkmn = true;
                                break;
                            }
                        }
                        if(found_pkmn){
                            break;
                        }
                    }
                    if(found_pkmn){
                        break;
                    }
                }
            }
        }
    }

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);

}

}
}
}

