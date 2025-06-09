/*  Tera Battle Menus
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSV_TeraBattleMenus.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


TeraBattleMenuDetector::TeraBattleMenuDetector(Color color)
    : m_callouts_button(color, WhiteButton::ButtonMinus, {0.35, 0.87, 0.30, 0.05})
    , m_arrow(color, GradientArrowType::RIGHT, {0.75, 0.62, 0.05, 0.35})
{}
void TeraBattleMenuDetector::make_overlays(VideoOverlaySet& items) const{
    m_callouts_button.make_overlays(items);
    m_arrow.make_overlays(items);
}
bool TeraBattleMenuDetector::detect(const ImageViewRGB32& screen){
    if (!m_callouts_button.detect(screen)){
//        cout << "status button" << endl;
        return false;
    }
    if (!m_arrow.detect(screen)){
//        cout << "arrow" << endl;
        return false;
    }
    return true;
}
int8_t TeraBattleMenuDetector::detect_slot(const ImageViewRGB32& screen){
    if (!m_callouts_button.detect(screen)){
//        cout << "status button" << endl;
        return -1;
    }

    ImageFloatBox box;
    if (!m_arrow.detect(box, screen)){
//        cout << "arrow" << endl;
//        screen.save("test.png");
        return -1;
    }

    double y = box.y + box.height * 0.5;
    double slot = (y - 0.761111) / 0.0814815;
//    cout << "slot = " << slot << endl;
    return (int8_t)(slot + 0.5);
}
bool TeraBattleMenuDetector::move_to_slot(VideoStream& stream, ProControllerContext& context, uint8_t slot){
    if (slot > 2){
        return false;
    }
    for (size_t attempts = 0;; attempts++){
        context.wait_for_all_requests();
        VideoSnapshot screen = stream.video().snapshot();
        int8_t current_slot = detect_slot(screen);
        if (current_slot < 0 || current_slot > 2){
            stream.log("TeraBattleMenuDetector::move_to_slot(): Unable to detect slot.", COLOR_RED);
            return false;
        }
        if (attempts > 10){
            stream.log("TeraBattleMenuDetector::move_to_slot(): Failed to move slot after 10 attempts.", COLOR_RED);
            return false;
        }

        uint8_t diff = (3 + slot - (uint8_t)current_slot) % 3;
        switch (diff){
        case 0:
            return true;
        case 1:
            pbf_press_dpad(context, DPAD_DOWN, 20, 30);
            continue;
        case 2:
            pbf_press_dpad(context, DPAD_UP, 20, 30);
            continue;
        }
    }
}





CheerSelectDetector::CheerSelectDetector(Color color)
    : m_info_button(color, WhiteButton::ButtonY, {0.35, 0.90, 0.30, 0.08})
    , m_arrow(color, GradientArrowType::RIGHT, {0.705, 0.710, 0.050, 0.260})
{}
void CheerSelectDetector::make_overlays(VideoOverlaySet& items) const{
    m_info_button.make_overlays(items);
    m_arrow.make_overlays(items);
}
bool CheerSelectDetector::detect(const ImageViewRGB32& screen){
    if (m_info_button.detect(screen)){
//        cout << "status" << endl;
        return false;
    }
    if (!m_arrow.detect(screen)){
//        cout << "arrow" << endl;
        return false;
    }
    return true;
}
int8_t CheerSelectDetector::detect_slot(const ImageViewRGB32& screen){
    if (m_info_button.detect(screen)){
//        cout << "status" << endl;
        return false;
    }

    ImageFloatBox box;
    if (!m_arrow.detect(box, screen)){
//        cout << "arrow" << endl;
        return false;
    }

    double y = box.y + box.height * 0.5;
    y = (y - 0.758333) / 0.082639;
//    cout << "y = " << y << endl;

    return (int8_t)(y + 0.5);
}
bool CheerSelectDetector::move_to_slot(VideoStream& stream, ProControllerContext& context, uint8_t slot){
    if (slot > 2){
        return false;
    }
    for (size_t attempts = 0;; attempts++){
        context.wait_for_all_requests();
        VideoSnapshot screen = stream.video().snapshot();
        int8_t current_slot = detect_slot(screen);
        if (current_slot < 0 || current_slot > 2){
            stream.log("CheerSelectDetector::move_to_slot(): Unable to detect slot.", COLOR_RED);
            return false;
        }
        if (attempts > 10){
            stream.log("CheerSelectDetector::move_to_slot(): Failed to move slot after 10 attempts.", COLOR_RED);
            return false;
        }

        uint8_t diff = (3 + slot - (uint8_t)current_slot) % 3;
        switch (diff){
        case 0:
            return true;
        case 1:
            pbf_press_dpad(context, DPAD_DOWN, 20, 30);
            continue;
        case 2:
            pbf_press_dpad(context, DPAD_UP, 20, 30);
            continue;
        }
    }
}





TeraTargetSelectDetector::TeraTargetSelectDetector(Color color)
    : m_opponent(color, GradientArrowType::DOWN, {0.45, 0.07, 0.10, 0.10})
    , m_player0(color, GradientArrowType::DOWN, {0.20, 0.46, 0.10, 0.10})
    , m_player1(color, GradientArrowType::DOWN, {0.37, 0.46, 0.10, 0.10})
    , m_player2(color, GradientArrowType::DOWN, {0.53, 0.46, 0.10, 0.10})
    , m_player3(color, GradientArrowType::DOWN, {0.70, 0.46, 0.10, 0.10})
{}
void TeraTargetSelectDetector::make_overlays(VideoOverlaySet& items) const{
    m_opponent.make_overlays(items);
    m_player0.make_overlays(items);
    m_player1.make_overlays(items);
    m_player2.make_overlays(items);
    m_player3.make_overlays(items);
}
bool TeraTargetSelectDetector::detect(const ImageViewRGB32& screen){
    if (m_opponent.detect(screen)){
        return true;
    }
    if (m_player0.detect(screen)){
        return true;
    }
    if (m_player1.detect(screen)){
        return true;
    }
    if (m_player2.detect(screen)){
        return true;
    }
    if (m_player3.detect(screen)){
        return true;
    }
    return false;
}
int8_t TeraTargetSelectDetector::detect_slot(const ImageViewRGB32& screen){
    if (m_opponent.detect(screen)){
        return 0;
    }
    if (m_player0.detect(screen)){
        return 1;
    }
    if (m_player1.detect(screen)){
        return 2;
    }
    if (m_player2.detect(screen)){
        return 3;
    }
    if (m_player3.detect(screen)){
        return 4;
    }
    return -1;
}
bool TeraTargetSelectDetector::move_to_slot(VideoStream& stream, ProControllerContext& context, uint8_t slot){
    if (slot > 4){
        return false;
    }
    for (size_t attempts = 0;; attempts++){
        context.wait_for_all_requests();
        VideoSnapshot screen = stream.video().snapshot();
        int8_t current_slot = detect_slot(screen);
        if (current_slot < 0 || current_slot > 4){
            stream.log("TargetSelectDetector::move_to_slot(): Unable to detect slot.", COLOR_RED);
            return false;
        }
        if (attempts > 10){
            stream.log("TargetSelectDetector::move_to_slot(): Failed to move slot after 10 attempts.", COLOR_RED);
            return false;
        }

        if (current_slot == 0 && slot != 0){
            pbf_press_dpad(context, DPAD_DOWN, 20, 30);
            continue;
        }
        if (current_slot != 0 && slot == 0){
            pbf_press_dpad(context, DPAD_UP, 20, 30);
            continue;
        }

        uint8_t diff = (4 + slot - (uint8_t)current_slot) % 4;
        switch (diff){
        case 0:
            return true;
        case 1:
            pbf_press_dpad(context, DPAD_RIGHT, 20, 30);
            continue;
        case 2:
            pbf_press_dpad(context, DPAD_RIGHT, 20, 30);
            pbf_press_dpad(context, DPAD_RIGHT, 20, 30);
            continue;
        case 3:
            pbf_press_dpad(context, DPAD_LEFT, 20, 30);
            continue;
        }
    }
}





TeraCatchDetector::TeraCatchDetector(Color color)
    : m_color(color)
    , m_callouts_button(color, WhiteButton::ButtonMinus, {0.35, 0.87, 0.30, 0.05})
{
    m_button[0] = {0.801, 0.818, 0.005, 0.047};
    m_button[1] = {0.801, 0.900, 0.005, 0.047};
    m_box_right[0] = {0.95, 0.810, 0.02, 0.06};
    m_box_right[1] = {0.95, 0.892, 0.02, 0.06};
    m_arrow.emplace_back(color, GradientArrowType::RIGHT, ImageFloatBox(0.75, 0.800, 0.08, 0.09));
    m_arrow.emplace_back(color, GradientArrowType::RIGHT, ImageFloatBox(0.75, 0.878, 0.08, 0.09));
}
void TeraCatchDetector::make_overlays(VideoOverlaySet& items) const{
    for (int c = 0; c < 2; c++){
        items.add(m_color, m_button[c]);
        items.add(m_color, m_box_right[c]);
        m_arrow[c].make_overlays(items);
    }
}

bool TeraCatchDetector::detect_slot(const ImageViewRGB32& screen, size_t index){
    ImageStats button = image_stats(extract_box_reference(screen, m_button[index]));
//    cout << button.average << button.stddev << endl;
//    extract_box_reference(screen, m_button).save("button.png");

    ImageStats yellow = image_stats(extract_box_reference(screen, m_box_right[index]));
//    cout << yellow.average << yellow.stddev << endl;
//    extract_box_reference(screen, m_slot0_box_right).save("yellow.png");

    bool button_ok = is_solid(button, {0.117281, 0.311767, 0.570951}, 0.20, 20) || is_black(button, 100, 20);
    bool yellow_ok = is_solid(yellow, {0.554348, 0.445652, 0.}, 0.15, 20);
    if (!button_ok && !yellow_ok){
//        cout << "button and yellow bad" << endl;
        return false;
    }

    if (!m_arrow[index].detect(screen)){
//        cout << "arrow bad" << endl;
        return false;
    }

    return true;
}
bool TeraCatchDetector::detect(const ImageViewRGB32& screen){
    if (m_callouts_button.detect(screen)){
        return false;
    }

    return detect_slot(screen, 0) || detect_slot(screen, 1);
}

bool TeraCatchDetector::move_to_slot(VideoStream& stream, ProControllerContext& context, uint8_t slot){
    if (slot > 1){
        return false;
    }

    for (size_t attempts = 0; attempts < 10; attempts++){
        context.wait_for_all_requests();
        VideoSnapshot screen = stream.video().snapshot();

        if (m_callouts_button.detect(screen)){
            stream.log("TeraCatchDetector::move_to_slot(): Unable to detect catch buttons.", COLOR_RED);
            return false;
        }

        bool slot0 = detect_slot(screen, 0);
        bool slot1 = detect_slot(screen, 1);
        if (slot0 == slot1){
            stream.log("TeraCatchDetector::move_to_slot(): Unable to detect catch buttons.", COLOR_RED);
            return false;
        }

        uint8_t current_slot = slot1 ? 1 : 0;

        if (current_slot == slot){
            return true;
        }

        pbf_press_dpad(context, DPAD_DOWN, 20, 30);
    }

    stream.log("TeraCatchDetector::move_to_slot(): Failed to move slot after 10 attempts.", COLOR_RED);
    return false;
}






}
}
}
