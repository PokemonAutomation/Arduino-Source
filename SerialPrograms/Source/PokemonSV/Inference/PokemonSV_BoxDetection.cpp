/*  Box Detection
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/ImageTools/SolidColorTest.h"
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



















}
}
}
