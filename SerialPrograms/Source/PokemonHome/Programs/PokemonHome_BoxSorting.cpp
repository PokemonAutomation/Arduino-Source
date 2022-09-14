/*  Box Reorder National Dex
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <sstream>
#include <map>
#include "CommonFramework/ImageTools/ImageFilter.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Language.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/OCR/OCR_NumberReader.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonFramework/OCR/OCR_TextMatcher.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Device.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Resources/Pokemon_PokemonSlugs.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
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
        )
    {
        PA_ADD_OPTION(BOX_NUMBER);
        PA_ADD_OPTION(VIDEO_DELAY);
        PA_ADD_OPTION(GAME_DELAY);
    }

bool go_to_first_slot(SingleSwitchProgramEnvironment& env, BotBaseContext& context, uint16_t video_delay){

    PokemonAutomation::ImageFloatBox Cursor_check(0.07, 0.15, 0.01, 0.01); //cursor position of the first slot of the box
    std::shared_ptr<const ImageRGB32> screen = env.console.video().snapshot();
    VideoOverlaySet BoxRender(env.console);

    BoxRender.add(COLOR_BLUE, Cursor_check);
    if(image_stats(extract_box_reference(*screen, Cursor_check)).average.r != 255){

        bool cursor_found = false;

        for (uint8_t lines = 0; lines < 7; lines++) {
            for (uint8_t colomn = 0; colomn < 5; colomn++) {
                pbf_press_dpad(context, DPAD_LEFT, 10, video_delay);
                context.wait_for_all_requests();
                screen = env.console.video().snapshot();

                if(image_stats(extract_box_reference(*screen, Cursor_check)).average.r == 255){
                    cursor_found = true;
                    break;
                }
            }
            if(!cursor_found){
                pbf_press_dpad(context, DPAD_UP, 10, video_delay);
                context.wait_for_all_requests();
                screen = env.console.video().snapshot();

                if(image_stats(extract_box_reference(*screen, Cursor_check)).average.r == 255){
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
  uint16_t colomn;
  uint16_t line;
};

void swap(std::vector<uint16_t>* arr, int pos1, int pos2){
    int temp;
    temp = (*arr)[pos1];
    (*arr)[pos1] = (*arr)[pos2];
    (*arr)[pos2] = temp;
}

int partition(std::vector<uint16_t>* arr, int low, int high, int pivot){
    int i = low;
    int j = low;
    while( i <= high){
        if((*arr)[i] > pivot){
            i++;
        }
        else{
            swap(arr,i,j);
            i++;
            j++;
        }
    }
    return j-1;
}

void quickSort(std::vector<uint16_t>* arr, int low, int high){
    if(low < high){
    int pivot = (*arr)[high];
    int pos = partition(arr, low, high, pivot);

    quickSort(arr, low, pos-1);
    quickSort(arr, pos+1, high);
    }
}

void move_cursor_to(SingleSwitchProgramEnvironment& env, BotBaseContext& context, uint16_t box_number_to, uint16_t colomn_to, uint16_t line_to, Cursor* cur_cursor, uint16_t uni_delay){

    std::ostringstream ss;
    ss << "The program think the cursor is at: " << cur_cursor->box << " " << cur_cursor->colomn << " " << cur_cursor->line;
    env.console.log(ss.str());
    ss.str("");

    if(box_number_to > cur_cursor->box){
        for (int i = 0; i < box_number_to-cur_cursor->box; ++i) {
            pbf_press_button(context, BUTTON_R, 10, uni_delay+30);
        }
    }else{
        for (int i = 0; i < cur_cursor->box-box_number_to; ++i) {
            pbf_press_button(context, BUTTON_L, 10, uni_delay+30);
        }
    }

    if(colomn_to > cur_cursor->colomn){
        for (int i = 0; i < colomn_to-cur_cursor->colomn; ++i) {
            pbf_press_dpad(context, DPAD_DOWN, 1, uni_delay);
        }
    }else{
        for (int i = 0; i < cur_cursor->colomn-colomn_to; ++i) {
            pbf_press_dpad(context, DPAD_UP, 1, uni_delay);
        }
    }

    if(line_to > cur_cursor->line){
        for (int i = 0; i < line_to-cur_cursor->line; ++i) {
            pbf_press_dpad(context, DPAD_RIGHT, 1, uni_delay);
        }
    }else{
        for (int i = 0; i < cur_cursor->line-line_to; ++i) {
            pbf_press_dpad(context, DPAD_LEFT, 1, uni_delay);
        }
    }
    cur_cursor->box = box_number_to;
    cur_cursor->colomn = colomn_to;
    cur_cursor->line = line_to;

    context.wait_for_all_requests();
}

void BoxSorting::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){

    PokemonAutomation::ImageFloatBox Select_check(0.495, 0.0045, 0.01, 0.005); // square color to check which mode is active
    PokemonAutomation::ImageFloatBox Dex_number_box(0.44, 0.245, 0.04, 0.04); //pokemon national dex number pos


    //3 dimensions vector that will store pokedex number of each slot
    std::vector<std::vector<std::vector<int16_t>>> box_data;

    //huge 2d vector that store coordinate of every box slot, I have no idea if this is optimizable or not
    std::vector<std::vector<PokemonAutomation::ImageFloatBox>> slot_list{
        {
            PokemonAutomation::ImageFloatBox(0.06, 0.2, 0.03, 0.057),
            PokemonAutomation::ImageFloatBox(0.132, 0.2, 0.03, 0.057),
            PokemonAutomation::ImageFloatBox(0.204, 0.2, 0.03, 0.057),
            PokemonAutomation::ImageFloatBox(0.276, 0.2, 0.03, 0.057),
            PokemonAutomation::ImageFloatBox(0.348, 0.2, 0.03, 0.057),
            PokemonAutomation::ImageFloatBox(0.42, 0.2, 0.03, 0.057),
        },

        {
            PokemonAutomation::ImageFloatBox(0.06, 0.303, 0.03, 0.057),
            PokemonAutomation::ImageFloatBox(0.132, 0.303, 0.03, 0.057),
            PokemonAutomation::ImageFloatBox(0.204, 0.303, 0.03, 0.057),
            PokemonAutomation::ImageFloatBox(0.276, 0.303, 0.03, 0.057),
            PokemonAutomation::ImageFloatBox(0.348, 0.303, 0.03, 0.057),
            PokemonAutomation::ImageFloatBox(0.42, 0.303, 0.03, 0.057),
        },

        {
            PokemonAutomation::ImageFloatBox(0.06, 0.303+0.1035, 0.03, 0.057),
            PokemonAutomation::ImageFloatBox(0.132, 0.303+0.1035, 0.03, 0.057),
            PokemonAutomation::ImageFloatBox(0.204, 0.303+0.1035, 0.03, 0.057),
            PokemonAutomation::ImageFloatBox(0.276, 0.303+0.1035, 0.03, 0.057),
            PokemonAutomation::ImageFloatBox(0.348, 0.303+0.1035, 0.03, 0.057),
            PokemonAutomation::ImageFloatBox(0.42, 0.303+0.1035, 0.03, 0.057),
        },

        {
            PokemonAutomation::ImageFloatBox(0.06, 0.303+0.1035*2, 0.03, 0.057),
            PokemonAutomation::ImageFloatBox(0.132, 0.303+0.1035*2, 0.03, 0.057),
            PokemonAutomation::ImageFloatBox(0.204, 0.303+0.1035*2, 0.03, 0.057),
            PokemonAutomation::ImageFloatBox(0.276, 0.303+0.1035*2, 0.03, 0.057),
            PokemonAutomation::ImageFloatBox(0.348, 0.303+0.1035*2, 0.03, 0.057),
            PokemonAutomation::ImageFloatBox(0.42, 0.303+0.1035*2, 0.03, 0.057),
        },

        {
            PokemonAutomation::ImageFloatBox(0.06, 0.303+0.1035*3, 0.03, 0.057),
            PokemonAutomation::ImageFloatBox(0.132, 0.303+0.1035*3, 0.03, 0.057),
            PokemonAutomation::ImageFloatBox(0.204, 0.303+0.1035*3, 0.03, 0.057),
            PokemonAutomation::ImageFloatBox(0.276, 0.303+0.1035*3, 0.03, 0.057),
            PokemonAutomation::ImageFloatBox(0.348, 0.303+0.1035*3, 0.03, 0.057),
            PokemonAutomation::ImageFloatBox(0.42, 0.303+0.1035*3, 0.03, 0.057)
        }
    };

    uint16_t video_delay = VIDEO_DELAY; //delay for every input that need video feedback, user will be able to modify this to enhance capture card delay compatibility

    uint16_t uni_delay = GAME_DELAY; //delay for non video feedback, this way I can go as fast as pokemon home can handle movement when needed

    uint16_t nb_box_choosen = BOX_NUMBER;

    Cursor cur_cursor{static_cast<uint16_t>(nb_box_choosen-1), 0, 0};

    std::shared_ptr<const ImageRGB32> screen = env.console.video().snapshot();

    VideoOverlaySet BoxRender(env.console);

    std::ostringstream ss;

    //getting out of every possible menu to make sure the program work no matter where you start it in your pokemon home
    if(image_stats(extract_box_reference(*screen, Select_check)).average.r != 255){
        for (int var = 0; var < 5; ++var) {
            pbf_press_button(context, BUTTON_B, 10, uni_delay+10);
        }
        context.wait_for_all_requests();
    }

    // switching cursor mode to be able to select summary and to make next detection easier
    BoxRender.add(COLOR_BLUE, Select_check);
    while(image_stats(extract_box_reference(*screen, Select_check)).average.r != 255){
        pbf_press_button(context, BUTTON_ZR, 10, video_delay+20); //additional delay because this animation is slower than the rest
        context.wait_for_all_requests();
        screen = env.console.video().snapshot();
    }
    BoxRender.clear();

    for (int box_nb = 0; box_nb < nb_box_choosen; ++box_nb) {

        if(box_nb != 0){
            pbf_press_button(context, BUTTON_R, 10, video_delay+100);
            context.wait_for_all_requests();
        }

        // Moving the cursor until it goes to the first slot
        if(!go_to_first_slot(env, context, video_delay)){
            env.console.log("ERROR: Could not move cursor to the first slot, please consider adjusting delay\n", COLOR_RED);
            return;
        }
        context.wait_for_all_requests();
        screen = env.console.video().snapshot();

        // Box grid to find empty slots (red boxes) and fill box_data with value to check or not for pokemon dex number
        box_data.push_back(std::vector<std::vector<int16_t>>());
        ss << "\n";
        for (uint8_t i = 0; i < slot_list.size(); i++) {

            box_data[box_nb].push_back(std::vector<int16_t>());

            for (uint8_t j = 0; j < slot_list.at(i).size(); j++) {

                FloatPixel current_box_value = image_stats(extract_box_reference(*screen, slot_list[i][j])).average;

                if(current_box_value.r >= 239 && current_box_value.g == 255 && current_box_value.b == 255){
                    BoxRender.add(COLOR_RED, slot_list[i][j]);
                    box_data[box_nb][i].push_back(9999);
                    ss << "❌ " ;
                }else{
                    BoxRender.add(COLOR_GREEN, slot_list[i][j]);
                    box_data[box_nb][i].push_back(0);
                    ss << "✅ " ;
                }
            }
            ss << "\n";
        }

        env.console.log(ss.str());
        ss.str("");

        bool find_first_poke = false;

        for (int colomn = 0; colomn < 5; ++colomn) {
            for (int line = 0; line < 6; ++line) {
                if(box_data[box_nb][colomn][line] != 9999){
                    find_first_poke = true;
                }
                if(!find_first_poke){
                    pbf_press_dpad(context, DPAD_RIGHT, 10, uni_delay);
                }
            }
            if(!find_first_poke){
                pbf_press_dpad(context, DPAD_DOWN, 10, uni_delay);
            }
        }

        pbf_press_button(context, BUTTON_A, 10, uni_delay);
        context.wait_for_all_requests();
        BoxRender.clear();
        pbf_press_dpad(context, DPAD_DOWN, 10, uni_delay);
        pbf_press_button(context, BUTTON_A, 10, video_delay+150);
        context.wait_for_all_requests();

        BoxRender.add(COLOR_RED, Dex_number_box);

        for (int colomn = 0; colomn < 5; ++colomn) {
            for (int line = 0; line < 6; ++line) {

                if(box_data[box_nb][colomn][line] == 0){
                    screen = env.console.video().snapshot();
                    ImageRGB32 image = to_blackwhite_rgb32_range(
                        extract_box_reference(*screen, Dex_number_box),
                        0xff808080, 0xffffffff, true
                    );
                    int DexNumber = OCR::read_number(env.console, image);
                    box_data[box_nb][colomn][line] = DexNumber;
                    pbf_press_button(context, BUTTON_R, 10, video_delay+15);
                    context.wait_for_all_requests();
                }
            }
        }

        BoxRender.clear();

        ss << std::endl;
        for (int colomn = 0; colomn < 5; ++colomn) {
            for (int line = 0; line < 6; ++line) {
                ss << box_data[box_nb][colomn][line] << " ";
            }
            ss << std::endl;
        }

        env.console.log(ss.str());
        ss.str("");
        pbf_press_button(context, BUTTON_B, 10, video_delay+200);
        BoxRender.clear();
        context.wait_for_all_requests();

        go_to_first_slot(env, context, video_delay);
        context.wait_for_all_requests();
    }
    move_cursor_to(env, context, 0, 0, 0, &cur_cursor, uni_delay);
    context.wait_for_all_requests();

    std::vector<uint16_t> flat_box;

    for (int box_nb = 0; box_nb < int(box_data.size()); ++box_nb) {
        for (int colomn = 0; colomn < 5; ++colomn) {
            for (int line = 0; line < 6; ++line) {
                flat_box.push_back(box_data[box_nb][colomn][line]);
            }
        }
    }

    quickSort(&flat_box, 0, int(flat_box.size()-1));

    for (auto const var : flat_box) {
        ss << var << " ";
    }
    env.console.log(ss.str());
    ss.str("");

    //move_cursor_to(env, context, 7, 3, 4, 0, 0, 0, uni_delay);

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

    for (auto const &box : box_sorted) {
        for (auto const &colomn : box) {
            for (auto const line : colomn) {
                ss << line << " ";
            }
            ss << std::endl;
        }
        ss << std::endl;
    }
    env.console.log(ss.str());
    ss.str("");


    //this need to be separated into functions

    for (int box_nb = 0; box_nb < int(box_sorted.size()); ++box_nb) {
        for (int colomn = 0; colomn < 5; ++colomn) {
            for (int line = 0; line < 6; ++line) {
                bool found_pkmn = false;
                for (int box_nb_s = 0; box_nb_s < int(box_data.size()); ++box_nb_s) {
                    for (int colomn_s = 0; colomn_s < 5; ++colomn_s) {
                        for (int line_s = 0; line_s < 6; ++line_s) {
                            if(box_sorted[box_nb][colomn][line] == box_data[box_nb_s][colomn_s][line_s] && box_data[box_nb_s][colomn_s][line_s] != box_data[box_nb][colomn][line]){
                                ss << "Moving cursor to " << box_nb_s <<" "<< colomn_s <<" "<< line_s;
                                env.console.log(ss.str());
                                ss.str("");
                                move_cursor_to(env, context, box_nb_s, colomn_s, line_s, &cur_cursor, uni_delay);
                                pbf_press_button(context, BUTTON_Y, 10, uni_delay+30);
                                ss << "Moving "<< box_data[box_nb_s][colomn_s][line_s] <<" to " << box_nb <<" "<< colomn <<" "<< line;
                                env.console.log(ss.str());
                                ss.str("");
                                move_cursor_to(env, context, box_nb, colomn, line, &cur_cursor, uni_delay);
                                pbf_press_button(context, BUTTON_Y, 10, uni_delay+30);
                                context.wait_for_all_requests();
                                box_data[box_nb_s][colomn_s][line_s] = box_data[box_nb][colomn][line];
                                box_data[box_nb][colomn][line] = 0;
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

}

}
}
}

