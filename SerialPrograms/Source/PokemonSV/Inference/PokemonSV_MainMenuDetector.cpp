/*  Main Menu Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV_MainMenuDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


MainMenuDetector::MainMenuDetector(Color color)
    : m_color(color)
    , m_bottom(0.10, 0.94, 0.40, 0.05)
    , m_arrow_left(color, GradientArrowType::RIGHT, {0.02, 0.10, 0.05, 0.90})
    , m_arrow_right(color, GradientArrowType::RIGHT, {0.67, 0.20, 0.05, 0.50})
    , m_dlc_icon(color, GradientArrowType::RIGHT, {0.67, 0.76, 0.05, 0.10})
{}
void MainMenuDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_bottom);
    m_arrow_left.make_overlays(items);
    m_arrow_right.make_overlays(items);
    m_dlc_icon.make_overlays(items);
}
bool MainMenuDetector::detect(const ImageViewRGB32& screen){
    //  Disambiguate against the Poke Portal.
    ImageStats bottom = image_stats(extract_box_reference(screen, m_bottom));
//    cout << bottom.average << bottom.stddev << endl;
    if (is_solid(bottom, {0.582218, 0.417782, 0.})){
        return false;
    }

    //  Check the arrows.
    if (m_arrow_left.detect(screen)){
        return true;
    }
    if (m_arrow_right.detect(screen)){
        return true;
    }
    if (m_dlc_icon.detect(screen)){
        return true;
    }

    return false;
}

std::pair<MenuSide, int> MainMenuDetector::detect_location(const ImageViewRGB32& screen){
    //  Disambiguate against the Poke Portal.
    ImageStats bottom = image_stats(extract_box_reference(screen, m_bottom));
    if (is_solid(bottom, {0.582218, 0.417782, 0.})){
        return {MenuSide::NONE, 0};
    }

    //  Check the arrows.
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
    if (m_dlc_icon.detect(screen)){
        return {MenuSide::RIGHT, 6};
    }

    return {MenuSide::NONE, 0};
}


bool MainMenuDetector::move_cursor(
    const ProgramInfo& info,
    VideoStream& stream, ProControllerContext& context,
    MenuSide side, int row, bool fast
){
    if (side == MenuSide::NONE){
        throw InternalProgramError(
            &stream.logger(), PA_CURRENT_FUNCTION,
            "MainMenuDetector::move_cursor() called with MenuSide::NONE."
        );
    }

    size_t consecutive_detection_fails = 0;
    size_t moves = 0;
    bool target_reached = false;
    while (true){
        context.wait_for_all_requests();
        VideoSnapshot screen = stream.video().snapshot();
        std::pair<MenuSide, int> current = this->detect_location(screen);

        {
            std::string text = "Current Location: ";
            switch (current.first){
            case MenuSide::NONE:
                text += "?";
                break;
            case MenuSide::LEFT:
                text += "Left";
                break;
            case MenuSide::RIGHT:
                text += "Right";
                break;
            }
            text += " " + std::to_string(current.second);
            stream.log(text);
        }

        //  Failed to detect menu.
        if (current.first == MenuSide::NONE){
            consecutive_detection_fails++;
            if (consecutive_detection_fails > 10){
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "MainMenuDetector::move_cursor(): Unable to detect menu.",
                    stream,
                    screen
                );
            }
            context.wait_for(std::chrono::milliseconds(50));
            continue;
        }
        consecutive_detection_fails = 0;

        if (moves >= 20){
            stream.log("Unable to move to target after 20 moves.", COLOR_RED);
            return false;
        }

        //  We're done!
        if (current.first == side && current.second == row){
            if (target_reached || fast){
                return true;
            }else{
                //  Don't return yet. Wait a second to make sure the video is
                //  in steady state before we return.
                target_reached = true;
                context.wait_for(std::chrono::seconds(1));
                continue;
            }
        }
        target_reached = false;

        moves++;

        //  Wrong side.
        if (current.first != side){
            if (current.first == MenuSide::LEFT){
                pbf_press_dpad(context, DPAD_RIGHT, 20, 10);
            }else{
                pbf_press_dpad(context, DPAD_LEFT, 20, 10);
            }
            continue;
        }

        int rows = side == MenuSide::LEFT ? 7 : 6;

//        cout << "current = " << current.second << endl;
//        cout << "target  = " << row << endl;

        int diff = (rows + current.second - row) % rows;
//        cout << "diff = " << diff << endl;
        if (diff < 4){
            if (fast){
                for (int c = 0; c < diff - 1; c++){
                    pbf_press_dpad(context, DPAD_UP, 10, 10);
                }
            }
            pbf_press_dpad(context, DPAD_UP, 20, 10);
        }else{
            if (fast){
                diff = rows - diff;
                for (int c = 0; c < diff - 1; c++){
                    pbf_press_dpad(context, DPAD_DOWN, 10, 10);
                }
            }
            pbf_press_dpad(context, DPAD_DOWN, 20, 10);
        }
    }
}







}
}
}
