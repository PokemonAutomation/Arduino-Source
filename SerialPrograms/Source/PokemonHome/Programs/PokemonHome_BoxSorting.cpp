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

//ImageViewRGB32 frame = extract_box_reference(*screen, region);
//ImageStats lacouleur = image_stats(frame);
//Logger& logger = env.console;

/*
    ImageRGB32 image = to_blackwhite_rgb32_range(
        extract_box_reference(*screen, timer),
        0xff808080, 0xffffffff, true
    );
    int DexNumber = OCR::read_number(env.console, image);

    std::ostringstream ss;
    ss << image_stats(extract_box_reference(*screen, Cursor_check)).average;
    env.console.log(ss.str());
*/

BoxSorting::BoxSorting(){}

void BoxSorting::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){

    PokemonAutomation::ImageFloatBox Select_check(0.495, 0.0045, 0.01, 0.005); // square color to check which mode is active
    PokemonAutomation::ImageFloatBox Cursor_check(0.07, 0.15, 0.01, 0.01); //cursor position of the first slot of the box
    PokemonAutomation::ImageFloatBox Dex_number_box(0.4475, 0.255, 0.035, 0.028); //pokemon national dex number pos


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

    uint16_t avg_delay = 20; //universal delay for every input that need video feedback, user will be able to modify this to enhance capture card delay compatibility

    std::shared_ptr<const ImageRGB32> screen = env.console.video().snapshot();

    VideoOverlaySet BoxRender(env.console);

    std::ostringstream ss;

    // switching cursor mode to be able to select summary and to make next detection easier

    BoxRender.add(COLOR_BLUE, Select_check);
    while(image_stats(extract_box_reference(*screen, Select_check)).average.r != 255){
        pbf_press_button(context, BUTTON_ZR, 10, avg_delay+20); //additional delay because this animation is slower than the rest
        context.wait_for_all_requests();
        screen = env.console.video().snapshot();
    }
    BoxRender.clear();

    // Moving the cursor until it goes to the first slot

    BoxRender.add(COLOR_BLUE, Cursor_check);
    if(image_stats(extract_box_reference(*screen, Cursor_check)).average.r != 255){

        bool cursor_found = false;

        for (uint8_t lines = 0; lines < 7; lines++) {
            for (uint8_t colomn = 0; colomn < 5; colomn++) {
                pbf_press_dpad(context, DPAD_LEFT, 10, avg_delay);
                context.wait_for_all_requests();
                screen = env.console.video().snapshot();

                if(image_stats(extract_box_reference(*screen, Cursor_check)).average.r == 255){
                    cursor_found = true;
                    break;
                }
            }
            if(!cursor_found){
                pbf_press_dpad(context, DPAD_UP, 10, avg_delay);
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
            //needs to show error here but should never happend since color check should break program above
            ss << "ERROR: Program failed to move the cursor to the first slot, consider doing it manualy and/or adjust color/delay";
            env.console.log(ss.str());
            return;
        }
    }
    BoxRender.clear();

    // Box grid to find empty slots (red boxes)
    box_data.push_back(std::vector<std::vector<int16_t>>());
    box_data.at(0).push_back(std::vector<int16_t>());
    ss << "\n";
    for (uint8_t i = 0; i < slot_list.size(); i++) {
        for (uint8_t j = 0; j < slot_list.at(i).size(); j++) {
            if(image_stats(extract_box_reference(*screen, slot_list[i][j])).average.r >= 239 && image_stats(extract_box_reference(*screen, slot_list[i][j])).average.g == 255 && image_stats(extract_box_reference(*screen, slot_list[i][j])).average.b == 255){
                BoxRender.add(COLOR_RED, slot_list[i][j]);
                ss << "❌ " ;
                box_data.at(0).at(i).push_back(-1);
            }else{
                BoxRender.add(COLOR_GREEN, slot_list[i][j]);
                ss << "✅ " ;
                box_data.at(0).at(i).push_back(0);
            }
        }
        ss << "\n";
    }

    env.console.log(ss.str());
    ss.str("");

    pbf_press_button(context, BUTTON_A, 10, avg_delay);
    context.wait_for_all_requests();
    BoxRender.clear();
    pbf_press_dpad(context, DPAD_DOWN, 10, avg_delay);
    pbf_press_button(context, BUTTON_A, 10, avg_delay+150);
    context.wait_for_all_requests();
    screen = env.console.video().snapshot();
    BoxRender.add(COLOR_RED, Dex_number_box);
    ImageRGB32 image = to_blackwhite_rgb32_range(
        extract_box_reference(*screen, Dex_number_box),
        0xff808080, 0xffffffff, true
    );
    int DexNumber = OCR::read_number(env.console, image);
    ss << DexNumber ;
    env.console.log(ss.str());
    pbf_press_button(context, BUTTON_B, 10, avg_delay);
    BoxRender.clear();

    //BoxRender.add(COLOR_RED, timer);
    //BoxRender.add(COLOR_BLUE, Select_check);

    //pbf_press_dpad(context, DPAD_DOWN, 10, 5);

    //pbf_press_button(context, BUTTON_B, 10, 1000);
    //context.wait_for_all_requests();

}


}
}
}

