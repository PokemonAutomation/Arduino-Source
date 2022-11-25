/*  Main Menu Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
//#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "PokemonSV_GradientArrowDetector.h"
#include "PokemonSV_MainMenuDetector.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


MainMenuDetector::MainMenuDetector(Color color)
    : m_color(color)
    , m_arrow_left(color, GradientArrowType::RIGHT, {0.02, 0.10, 0.05, 0.90})
    , m_arrow_right(color, GradientArrowType::RIGHT, {0.67, 0.20, 0.05, 0.50})
{}
void MainMenuDetector::make_overlays(VideoOverlaySet& items) const{
    m_arrow_left.make_overlays(items);
    m_arrow_right.make_overlays(items);
}
bool MainMenuDetector::detect(const ImageViewRGB32& screen) const{
    if (m_arrow_left.detect(screen)){
        return true;
    }
    if (m_arrow_right.detect(screen)){
        return true;
    }
    return false;
}

std::pair<MenuSide, int> MainMenuDetector::detect_location(const ImageViewRGB32& screen) const{
    ImageFloatBox box;
    if (m_arrow_left.detect(box, screen)){
        if (box.y > 0.85){
            return {MenuSide::LEFT, 6};
        }
        int slot = (int)((box.y - 0.172222) / 0.116482 + 0.5);
        if (slot < 0){
            return {MenuSide::NONE, 0};
        }
        return {MenuSide::LEFT, slot};
    }
    if (m_arrow_right.detect(box, screen)){
//        cout << box.y << endl;
        int slot = (int)((box.y - 0.227778) / 0.074074 + 0.5);
        if (slot < 0){
            return {MenuSide::NONE, 0};
        }
        return {MenuSide::RIGHT, slot};
    }

    return {MenuSide::NONE, 0};
}


void MainMenuDetector::move_cursor(
    ConsoleHandle& console, BotBaseContext& context,
    MenuSide side, int row
) const{
    if (side == MenuSide::NONE){
        throw InternalProgramError(
            &console.logger(), PA_CURRENT_FUNCTION,
            "MainMenuDetector::move_cursor() called with MenuSide::NONE."
        );
    }

    size_t consecutive_fails = 0;
    while (true){
        context.wait_for_all_requests();
        VideoSnapshot screen = console.video().snapshot();
        std::pair<MenuSide, int> current = this->detect_location(screen);

        //  Failed to detect menu.
        if (current.first == MenuSide::NONE){
            consecutive_fails++;
            if (consecutive_fails > 10){
                throw OperationFailedException(console.logger(), "Unable to detect menu.");
            }
            context.wait_for(std::chrono::milliseconds(100));
            continue;
        }
        consecutive_fails = 0;

        //  Wrong side.
        if (current.first != side){
            if (current.first == MenuSide::LEFT){
                pbf_press_dpad(context, DPAD_RIGHT, 20, 105);
            }else{
                pbf_press_dpad(context, DPAD_LEFT, 20, 105);
            }
            continue;
        }

        //  We're done!
        if (current.second == row){
            return;
        }

        if (current.second > row){
            for (int c = current.second; c != row; c--){
                pbf_press_dpad(context, DPAD_UP, 20, 30);
            }
        }else{
            for (int c = current.second; c != row; c++){
                pbf_press_dpad(context, DPAD_DOWN, 20, 30);
            }
        }
    }
}







}
}
}
