/*  Home Box Sorting
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <sstream>
#include <map>
#include "CommonFramework/ImageTools/ImageFilter.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/OCR/OCR_NumberReader.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonFramework/OCR/OCR_TextMatcher.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_GameEntry.h"
#include "PokemonSwSh/Programs/ReleaseHelpers.h"
#include "PokemonHome_BoxSorting.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"

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
        FeedbackType::REQUIRED, false,
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

    PokemonAutomation::ImageFloatBox Cursor_check(0.07, 0.15, 0.01, 0.01); //cursor position of the first slot of the box
    std::shared_ptr<const ImageRGB32> screen = env.console.video().snapshot();
    PokemonAutomation::FloatPixel image_value = image_stats(extract_box_reference(*screen, Cursor_check)).average;
    env.console.log("Cursor color detection: " + image_value.to_string());
    VideoOverlaySet BoxRender(env.console);

    BoxRender.add(COLOR_BLUE, Cursor_check);
    if(image_value.r <= image_value.g + image_value.b){

        bool cursor_found = false;

        for (uint8_t lines = 0; lines < 7; lines++) {
            for (uint8_t column = 0; column < 5; column++) {
                pbf_press_dpad(context, DPAD_LEFT, 10, VIDEO_DELAY);
                context.wait_for_all_requests();
                screen = env.console.video().snapshot();
                image_value = image_stats(extract_box_reference(*screen, Cursor_check)).average;
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
                image_value = image_stats(extract_box_reference(*screen, Cursor_check)).average;
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



void swap(std::vector<uint16_t>* arr, int pos1, int pos2){
    int temp;
    temp = (*arr)[pos1];
    (*arr)[pos1] = (*arr)[pos2];
    (*arr)[pos2] = temp;
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

    PokemonAutomation::ImageFloatBox Select_check(0.495, 0.0045, 0.01, 0.005); // square color to check which mode is active
    PokemonAutomation::ImageFloatBox Dex_number_box(0.44, 0.245, 0.04, 0.04); //pokemon national dex number pos


    //3 dimensions vector that will store pokedex number of each slot
    std::vector<std::vector<std::vector<int16_t>>> box_data;

    Cursor cur_cursor{static_cast<uint16_t>(BOX_NUMBER-1), 0, 0};

    std::shared_ptr<const ImageRGB32> screen = env.console.video().snapshot();

    VideoOverlaySet BoxRender(env.console);

    std::ostringstream ss;

    PokemonAutomation::FloatPixel image_value = image_stats(extract_box_reference(*screen, Select_check)).average;

    env.console.log("Color detected from the select square: " + image_value.to_string());

    //if the correct color is not detected, getting out of every possible menu to make sure the program work no matter where you start it in your pokemon home
    BoxRender.add(COLOR_BLUE, Select_check);
    if(image_value.r <= image_value.g + image_value.b){
        for (int var = 0; var < 5; ++var) {
            pbf_press_button(context, BUTTON_B, 10, GAME_DELAY+10);
        }
        context.wait_for_all_requests();
        context.wait_for(std::chrono::milliseconds(VIDEO_DELAY));
        screen = env.console.video().snapshot();
        image_value = image_stats(extract_box_reference(*screen, Select_check)).average;
        env.console.log("Color detected from the select square: " + image_value.to_string());
        if(image_value.r <= image_value.g + image_value.b){
            for (int i = 0; i < 2; ++i) {
                pbf_press_button(context, BUTTON_ZR, 10, VIDEO_DELAY+20); //additional delay because this animation is slower than the rest
                context.wait_for_all_requests();
                screen = env.console.video().snapshot();
                image_value = image_stats(extract_box_reference(*screen, Select_check)).average;
                env.console.log("Color detected from the select square: " + image_value.to_string());
                if(image_value.r > image_value.g + image_value.b){
                    break;
                }else if(i==1){
                    env.console.log("ERROR: Could not find correct color mode please check color logs and timings\n", COLOR_RED);
                    return;
                }
            }
        }
    }

    BoxRender.clear();

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
        box_data.push_back(std::vector<std::vector<int16_t>>());

        ss << "\n";

        for (uint8_t i = 0; i < 5; i++) {

            box_data[box_nb].push_back(std::vector<int16_t>());

            for (uint8_t j = 0; j < 6; j++) {

                int current_box_value = image_stddev(extract_box_reference(*screen,  PokemonAutomation::ImageFloatBox(0.06+(0.072*j), 0.2+(0.1035*i), 0.03, 0.057))).sum();

                ss << current_box_value;

                //checking color to know if a pokemon is on the slot or not
                if(current_box_value < 15){
                    BoxRender.add(COLOR_RED, PokemonAutomation::ImageFloatBox(0.06+(0.072*j), 0.2+(0.1035*i), 0.03, 0.057));
                    box_data[box_nb][i].push_back(9999); //huge value to make sorting easier later
                    ss << "❌ " ;
                }else{
                    BoxRender.add(COLOR_GREEN, PokemonAutomation::ImageFloatBox(0.06+(0.072*j), 0.2+(0.1035*i), 0.03, 0.057));
                    box_data[box_nb][i].push_back(0); //0 value to know there is a pokemon here that needs a value
                    ss << "✅ " ;
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
                if(box_data[box_nb][column][line] != 9999){
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
        BoxRender.clear();
        pbf_press_dpad(context, DPAD_DOWN, 10, GAME_DELAY);
        pbf_press_button(context, BUTTON_A, 10, VIDEO_DELAY+150);
        context.wait_for_all_requests();

        BoxRender.add(COLOR_RED, Dex_number_box);

        //cycle through each summary of the current box and fill pokemon dex number in
        for (int column = 0; column < 5; ++column) {
            for (int line = 0; line < 6; ++line) {

                if(box_data[box_nb][column][line] == 0){
                    screen = env.console.video().snapshot();
                    ImageRGB32 image = to_blackwhite_rgb32_range(
                        extract_box_reference(*screen, Dex_number_box),
                        0xff808080, 0xffffffff, true
                    );
                    int DexNumber = OCR::read_number(env.console, image);
                    box_data[box_nb][column][line] = DexNumber;
                    pbf_press_button(context, BUTTON_R, 10, VIDEO_DELAY+15);
                    context.wait_for_all_requests();
                }
            }
        }

        BoxRender.clear();

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
        BoxRender.clear();
        context.wait_for_all_requests();

        go_to_first_slot(env, context, VIDEO_DELAY);
        context.wait_for_all_requests();
    }

    //making a flat list of the pokemons because turns out my original idea of 3d data is turning into a pain to use for sorting
    std::vector<uint16_t> flat_box;

    for (int box_nb = 0; box_nb < int(box_data.size()); ++box_nb) {
        for (int column = 0; column < 5; ++column) {
            for (int line = 0; line < 6; ++line) {
                flat_box.push_back(box_data[box_nb][column][line]);
            }
        }
    }

    //using sort to sort the flat copy of box data
    std::sort(flat_box.begin(), flat_box.end());

    for (auto const var : flat_box) {
        ss << var << " ";
    }
    env.console.log(ss.str());
    ss.str("");

    //make the flat sorted list 3d box grid again because why not now that I'm here
    std::vector<std::vector<std::vector<int16_t>>> box_sorted;

    for (int i = 0; i < int(flat_box.size()); ++i) {
        if(i%30==0){
            box_sorted.push_back(std::vector<std::vector<int16_t>>());
        }
        if(i%6==0){
            box_sorted[i/30].push_back(std::vector<int16_t>());
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
                                box_data[box_nb][column][line] = 0; //delete pokemon data to make sure it does not match again
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

