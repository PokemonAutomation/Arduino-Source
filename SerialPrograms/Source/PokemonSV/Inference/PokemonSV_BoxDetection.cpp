/*  Box Detection
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "PokemonSV_BoxDetection.h"

#include <iostream>
using std::cout;
using std::endl;;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


SomethingInBoxSlotDetector::SomethingInBoxSlotDetector(Color color, bool true_if_exists)
    : m_true_if_exists(true_if_exists)
    , m_color(color)
    , m_exists(0.985, 0.010, 0.010, 0.050)
{}
void SomethingInBoxSlotDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_exists);
}
bool SomethingInBoxSlotDetector::detect(const ImageViewRGB32& screen) const{
    ImageStats stats = image_stats(extract_box_reference(screen, m_exists));
    bool exists = is_solid(stats, {0.533473, 0.466527, 0.0});
//    cout << "exists = " << exists << endl;
    return exists == m_true_if_exists;
}




BoxSelectDetector::BoxSelectDetector(Color color)
    : m_color(color)
    , m_exists(color, true)
    , m_dialog(color)
    , m_gradient(color, GradientArrowType::RIGHT, {0.20, 0.17, 0.50, 0.10})
{}
void BoxSelectDetector::make_overlays(VideoOverlaySet& items) const{
    m_exists.make_overlays(items);
    m_dialog.make_overlays(items);
    m_gradient.make_overlays(items);
}
bool BoxSelectDetector::exists(const ImageViewRGB32& screen) const{
    return m_exists.detect(screen);
}
bool BoxSelectDetector::detect(const ImageViewRGB32& screen) const{
    if (!exists(screen)){
        return false;
    }
    if (!m_dialog.detect(screen)){
        return false;
    }
    if (!m_gradient.detect(screen)){
        return false;
    }
    return true;
}


BoxDetector::BoxDetector(Color color)
    : m_color(color)
    , m_party(color, GradientArrowType::DOWN, {0.140, 0.150, 0.050, 0.700})
    , m_box_change(color, GradientArrowType::DOWN, {0.405, 0.070, 0.050, 0.090})
    , m_all_boxes(color, GradientArrowType::DOWN, {0.295, 0.780, 0.050, 0.090})
    , m_search(color, GradientArrowType::DOWN, {0.510, 0.780, 0.050, 0.090})
    , m_slots(color, GradientArrowType::DOWN, {0.240, 0.160, 0.380, 0.550})
{}
void BoxDetector::make_overlays(VideoOverlaySet& items) const{
    m_party.make_overlays(items);
    m_box_change.make_overlays(items);
    m_all_boxes.make_overlays(items);
    m_search.make_overlays(items);
    m_slots.make_overlays(items);
}
bool BoxDetector::detect(const ImageViewRGB32& screen) const{
    if (m_party.detect(screen)){
        return true;
    }
    if (m_box_change.detect(screen)){
        return true;
    }
    if (m_all_boxes.detect(screen)){
        return true;
    }
    if (m_search.detect(screen)){
        return true;
    }
    if (m_slots.detect(screen)){
        return true;
    }
    return false;
}
std::pair<BoxCursorLocation, BoxCursorCoordinates> BoxDetector::detect_location(const ImageViewRGB32& screen) const{
    if (m_box_change.detect(screen)){
        return {BoxCursorLocation::BOX_CHANGE, {0, 0}};
    }
    if (m_all_boxes.detect(screen)){
        return {BoxCursorLocation::ALL_BOXES, {0, 0}};
    }
    if (m_search.detect(screen)){
        return {BoxCursorLocation::SEARCH, {0, 0}};
    }

    ImageFloatBox box;
    if (m_party.detect(box, screen)){
//        cout << box.y << endl;
        int slot = (int)((box.y - 0.175926) / 0.116296 + 0.5);
        if (slot < 0){
            return {BoxCursorLocation::NONE, {0, 0}};
        }
        return {BoxCursorLocation::PARTY, {(uint8_t)slot, 0}};
    }
    if (m_slots.detect(box, screen)){
//        cout << box.x << " " << box.y << endl;
        int x = (int)((box.x - 0.247917) / 0.065625 + 0.5);
        if (x < 0){
            return {BoxCursorLocation::NONE, {0, 0}};
        }
        int y = (int)((box.y - 0.175926) / 0.11713 + 0.5);
        if (x < 0){
            return {BoxCursorLocation::NONE, {0, 0}};
        }
        return {BoxCursorLocation::SLOTS, {(uint8_t)y, (uint8_t)x}};
    }

    return {BoxCursorLocation::NONE, {0, 0}};
}


bool BoxDetector::to_coordinates(int& x, int& y, BoxCursorLocation side, uint8_t row, uint8_t col) const{
    switch (side){
    case BoxCursorLocation::NONE:
        return false;
    case BoxCursorLocation::PARTY:
        x = -1;
        y = row;
        break;
    case BoxCursorLocation::BOX_CHANGE:
        x = 2;
        y = -1;
        break;
    case BoxCursorLocation::ALL_BOXES:
        x = 1;
        y = 5;
        break;
    case BoxCursorLocation::SEARCH:
        x = 4;
        y = 5;
        break;
    case BoxCursorLocation::SLOTS:
        x = col;
        y = row;
        break;
    }
    return true;
}
void BoxDetector::move_vertical(BotBaseContext& context, int current, int desired) const{
    int diff = (current - desired + 7) % 7;
//    cout << "diff = " << diff << endl;
    if (diff <= 3){
        for (int c = 0; c < diff; c++){
            pbf_press_dpad(context, DPAD_UP, 20, 30);
        }
    }else{
        for (int c = diff; c < 7; c++){
            pbf_press_dpad(context, DPAD_DOWN, 20, 30);
        }
    }
}
void BoxDetector::move_horizontal(BotBaseContext& context, int current, int desired) const{
    int diff = (current - desired + 7) % 7;
    if (diff <= 3){
        for (int c = 0; c < diff; c++){
            pbf_press_dpad(context, DPAD_LEFT, 20, 30);
        }
    }else{
        for (int c = diff; c < 7; c++){
            pbf_press_dpad(context, DPAD_RIGHT, 20, 30);
        }
    }
}


void BoxDetector::move_cursor(
    ConsoleHandle& console, BotBaseContext& context,
    BoxCursorLocation side, uint8_t row, uint8_t col
) const{
    int desired_x, desired_y;
    if (!to_coordinates(desired_x, desired_y, side, row, col)){
        throw InternalProgramError(
            &console.logger(), PA_CURRENT_FUNCTION,
            "BoxDetector::move_cursor() called with BoxCursorLocation::NONE."
        );
    }
//    cout << "desired_x = " << desired_x << ", desired_y = " << desired_y << endl;

    size_t consecutive_fails = 0;
    while (true){
        context.wait_for_all_requests();
        VideoSnapshot screen = console.video().snapshot();
        std::pair<BoxCursorLocation, BoxCursorCoordinates> current = this->detect_location(screen);

        int current_x, current_y;
        if (!to_coordinates(current_x, current_y, current.first, current.second.row, current.second.col)){
            consecutive_fails++;
            if (consecutive_fails > 10){
                throw OperationFailedException(console.logger(), "Unable to detect box system.");
            }
            context.wait_for(std::chrono::milliseconds(100));
            continue;
        }
//        cout << "current_x = " << current_x << ", current_y = " << current_y << endl;

        if (current_x == desired_x && current_y == desired_y){
//            cout << "done!" << endl;
            return;
        }

        //  If we're on the party, always move horizontally off it first.
        if (current_x == -1){
            if (desired_x == -1){
                if (row < current.second.row){
                    for (uint8_t r = current.second.row; r != row; r--){
                        pbf_press_dpad(context, DPAD_UP, 20, 30);
                    }
                }else{
                    for (uint8_t r = current.second.row; r != row; r--){
                        pbf_press_dpad(context, DPAD_DOWN, 20, 30);
                    }
                }
                continue;
            }
            if (desired_x < 3){
                pbf_press_dpad(context, DPAD_RIGHT, 20, 30);
            }else{
                pbf_press_dpad(context, DPAD_LEFT, 20, 30);
            }
            continue;
        }

        //  Otherwise, always move vertically first.
        if (current_y != desired_y){
            move_vertical(context, current_y, desired_y);
            continue;
        }

        //  Special case for bottom row.
        if (current_y == 5){
            int diff = (current_x - desired_x + 7) % 7;
            if (diff <= 3){
                pbf_press_dpad(context, DPAD_LEFT, 20, 30);
            }else{
                pbf_press_dpad(context, DPAD_RIGHT, 20, 30);
            }
            continue;
        }

        //  Now handle horizontals.
        move_horizontal(context, current_x, desired_x);
    }
}
















}
}
}
