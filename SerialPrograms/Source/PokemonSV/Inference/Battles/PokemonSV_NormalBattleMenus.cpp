/*  Normal Battle Menus
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/Tools/DebugDumper.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonSV_NormalBattleMenus.h"


// #include <iostream>
// using std::cout;
// using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



NormalBattleMenuDetector::NormalBattleMenuDetector(Color color)
    : m_status_button(color, WhiteButton::ButtonY, {0.35, 0.90, 0.30, 0.08})
    , m_arrow(color, GradientArrowType::RIGHT, {0.75, 0.62, 0.05, 0.35})
{}
void NormalBattleMenuDetector::make_overlays(VideoOverlaySet& items) const{
    m_status_button.make_overlays(items);
    m_arrow.make_overlays(items);
}
bool NormalBattleMenuDetector::detect(const ImageViewRGB32& screen){
    if (!m_status_button.detect(screen)){
//        cout << "status button" << endl;
        return false;
    }
    if (!m_arrow.detect(screen)){
        return false;
    }
    return true;
}
int8_t NormalBattleMenuDetector::detect_slot(const ImageViewRGB32& screen){
    if (!m_status_button.detect(screen)){
//        cout << "status button" << endl;
        return -1;
    }

    ImageFloatBox box;
    if (!m_arrow.detect(box, screen)){
        return -1;
    }

    double y = box.y + box.height * 0.5;
    double slot = (y - 0.67963) / 0.0814815;
//    cout << "slot = " << slot << endl;
    return (int8_t)(slot + 0.5);
}
bool NormalBattleMenuDetector::move_to_slot(VideoStream& stream, ProControllerContext& context, uint8_t slot){
    if (slot > 3){
        return false;
    }
    for (size_t attempts = 0;; attempts++){
        if (attempts > 10){
            stream.log("NormalBattleMenuDetector::move_to_slot(): Failed to move slot after 10 attempts.", COLOR_RED);
            return false;
        }

        context.wait_for_all_requests();

        VideoSnapshot screen = stream.video().snapshot();
        int8_t current_slot = detect_slot(screen);
        if (current_slot < 0 || current_slot > 3){
            stream.log("NormalBattleMenuDetector::move_to_slot(): Unable to detect slot.", COLOR_RED);
            context.wait_for(std::chrono::milliseconds(500));
            continue;
//            static int c = 0;
//            screen->save("bad-" + std::to_string(c++) + ".png");
//            return false;
        }

        uint8_t diff = (4 + slot - (uint8_t)current_slot) % 4;
        switch (diff){
        case 0:{
//            static int c = 0;
//            screen->save("good-" + std::to_string(c++) + ".png");
            return true;
        }
        case 1:
            pbf_press_dpad(context, DPAD_DOWN, 20, 30);
            continue;
        case 2:
            pbf_press_dpad(context, DPAD_DOWN, 20, 30);
            continue;
        case 3:
            pbf_press_dpad(context, DPAD_UP, 20, 30);
            continue;
        }
    }
}



std::set<std::string> read_singles_opponent(
    const ProgramInfo& info, VideoStream& stream, ProControllerContext& context,
    Language language
){
    VideoOverlaySet overlay = stream.overlay();

    ImageFloatBox name(0.422, 0.131, 0.120, 0.050);
    overlay.add(COLOR_RED, name);

    std::set<std::string> slugs;

    bool status_opened = false;
    bool battle_menu_seen = false;
    for (size_t c = 0; c < 10; c++){
        NormalBattleMenuWatcher battle_menu(COLOR_YELLOW);
        GradientArrowWatcher arrow(COLOR_BLUE, GradientArrowType::DOWN, {0.4, 0.1, 0.2, 0.5});
        context.wait_for_all_requests();
        int ret = wait_until(
            stream, context, std::chrono::seconds(5),
            {battle_menu, arrow}
        );
        context.wait_for(std::chrono::milliseconds(500));

        switch (ret){
        case 0:
            if (status_opened){
                stream.log("Detected battle menu...");
                return slugs;
            }
            stream.log("Detected battle menu. Opening status...");
            battle_menu_seen = true;
            pbf_press_button(context, BUTTON_Y, 20, 105);
            continue;

        case 1:
            if (!battle_menu_seen){
                stream.log("Detected status menu before pressing Y...", COLOR_RED);
//                dump_image(stream, info, "BattleMenuNotSeen", arrow.last_detected());
                continue;
            }
            if (status_opened){
                stream.log("Detected status menu (again)...", COLOR_RED);
            }else{
                stream.log("Detected status menu. Reading name...");
                status_opened = true;
                VideoSnapshot screen = stream.video().snapshot();
                OCR::StringMatchResult result = Pokemon::PokemonNameReader::instance().read_substring(
                    stream.logger(), language,
                    extract_box_reference(screen, name),
                    OCR::WHITE_TEXT_FILTERS()
                );
                for (auto& item : result.results){
                    slugs.insert(std::move(item.second.token));
                }
                if (slugs.empty()){
                    dump_image(stream.logger(), info, "UnableToReadName", screen);
                }
            }

            pbf_mash_button(context, BUTTON_B, 125);
            continue;

        default:
            stream.log("No recognized state. Mashing B...", COLOR_RED);
            pbf_mash_button(context, BUTTON_B, 250);
        }
    }

    OperationFailedException::fire(
        ErrorReport::SEND_ERROR_REPORT,
        "Unable to open status menu to read opponent name.",
        stream
    );
}





MoveSelectDetector::MoveSelectDetector(Color color)
    : m_info_button(color, WhiteButton::ButtonY, {0.35, 0.90, 0.30, 0.08})
    , m_arrow(color, GradientArrowType::RIGHT, {0.705, 0.550, 0.050, 0.410})
{}
void MoveSelectDetector::make_overlays(VideoOverlaySet& items) const{
    m_info_button.make_overlays(items);
    m_arrow.make_overlays(items);
}
bool MoveSelectDetector::detect(const ImageViewRGB32& screen){
    if (!m_info_button.detect(screen)){
//        cout << "status" << endl;
        return false;
    }
    if (!m_arrow.detect(screen)){
//        cout << "arrow" << endl;
        return false;
    }
    return true;
}
int8_t MoveSelectDetector::detect_slot(const ImageViewRGB32& screen){
    if (!m_info_button.detect(screen)){
//        cout << "status" << endl;
        return false;
    }

    ImageFloatBox box;
    if (!m_arrow.detect(box, screen)){
//        cout << "arrow" << endl;
        return false;
    }

    double y = box.y + box.height * 0.5;
    y = (y - 0.602778) / 0.103549;
//    cout << "y = " << y << endl;

    return (int8_t)(y + 0.5);
}
bool MoveSelectDetector::move_to_slot(VideoStream& stream, ProControllerContext& context, uint8_t slot){
    if (slot > 3){
        return false;
    }
    for (size_t attempts = 0;; attempts++){
        context.wait_for_all_requests();
        VideoSnapshot screen = stream.video().snapshot();
        int8_t current_slot = detect_slot(screen);
        if (current_slot < 0 || current_slot > 3){
            stream.log("MoveSelectDetector::move_to_slot(): Unable to detect slot.", COLOR_RED);
            return false;
        }
        if (attempts > 10){
            stream.log("MoveSelectDetector::move_to_slot(): Failed to move slot after 10 attempts.", COLOR_RED);
            return false;
        }

        uint8_t diff = (4 + slot - (uint8_t)current_slot) % 4;
        switch (diff){
        case 0:
            return true;
        case 1:
            pbf_press_dpad(context, DPAD_DOWN, 20, 30);
            continue;
        case 2:
            pbf_press_dpad(context, DPAD_DOWN, 20, 30);
            pbf_press_dpad(context, DPAD_DOWN, 20, 30);
            continue;
        case 3:
            pbf_press_dpad(context, DPAD_UP, 20, 30);
            continue;
        }
    }
}




TerastallizingDetector::TerastallizingDetector(Color color)
    : m_color(color)
    , m_box(0.62, 0.75, 0.03, 0.06)
{}
void TerastallizingDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}
bool TerastallizingDetector::detect(const ImageViewRGB32& screen){
    ImageStats box = image_stats(extract_box_reference(screen, m_box));
    // dump_debug_image(
    //             global_logger_command_line(), 
    //             "Test/TeraDetector", 
    //             "box", 
    //             extract_box_reference(screen, m_box));
    // cout << box.average.sum() << endl;

    return box.average.sum() > 600;
}




SwapMenuDetector::SwapMenuDetector(Color color)
    : m_arrow(color, GradientArrowType::RIGHT, { 0.02, 0.10, 0.05, 0.90 })
{}
void SwapMenuDetector::make_overlays(VideoOverlaySet& items) const{
    m_arrow.make_overlays(items);
}
bool SwapMenuDetector::detect(const ImageViewRGB32& screen){
    if (!m_arrow.detect(screen)){
        return false;
    }
    return true;
}
int8_t SwapMenuDetector::detect_slot(const ImageViewRGB32& screen) const{
    ImageFloatBox box;
    if (!m_arrow.detect(box, screen)){
        return -1;
    }

    int slot = (int)((box.y - 0.172222) / 0.116482 + 0.5);
    if (slot < 0){
        slot = 0;
    }
    //cout << "slot = " << slot << endl;
    return (int8_t)slot;
}
bool SwapMenuDetector::move_to_slot(VideoStream& stream, ProControllerContext& context, uint8_t slot) const{
    if (slot > 5){
        return false;
    }
    for (size_t attempts = 0;; attempts++){
        context.wait_for_all_requests();
        VideoSnapshot screen = stream.video().snapshot();
        int8_t current_slot = detect_slot(screen);
        if (current_slot < 0 || current_slot > 5){
            stream.log("SwapMenuDetector::move_to_slot(): Unable to detect slot.", COLOR_RED);
            return false;
        }
        if (attempts > 20){
            stream.log("SwapMenuDetector::move_to_slot(): Failed to move slot after 20 attempts.", COLOR_RED);
            return false;
        }

        uint8_t diff = (6 + slot - (uint8_t)current_slot) % 6;
        switch (diff){
        case 0:
            return true;
        case 1:
            pbf_press_dpad(context, DPAD_DOWN, 20, 30);
            continue;
        case 2:
            pbf_press_dpad(context, DPAD_DOWN, 20, 30);
            continue;
        case 3:
            pbf_press_dpad(context, DPAD_DOWN, 20, 30);
            continue;
        case 4:
            pbf_press_dpad(context, DPAD_DOWN, 20, 30);
            continue;
        case 5:
            pbf_press_dpad(context, DPAD_DOWN, 20, 30);
            continue;
        }
    }
}

WipeoutDetector::WipeoutDetector(Color color)
    : m_blackscreen(COLOR_RED, {0.1, 0.1, 0.8, 0.6})
    , m_dialog(color, true, DialogType::DIALOG_WHITE)
    , m_arrow_detector(COLOR_BLUE, {0.710, 0.850, 0.030, 0.042})
{}
void WipeoutDetector::make_overlays(VideoOverlaySet& items) const{
    m_blackscreen.make_overlays(items);
    m_dialog.make_overlays(items);
    m_arrow_detector.make_overlays(items);
}
bool WipeoutDetector::detect(const ImageViewRGB32& screen){
    if (!m_blackscreen.detect(screen)){
        return false;
    }
    
    if(!m_dialog.detect(screen)){
        return false;
    }

    return m_arrow_detector.detect(screen);
}


}
}
}
