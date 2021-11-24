/*  Dialog Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "CommonFramework/Inference/ImageTools.h"
#include "PokemonBDSP_DialogDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


ShortDialogDetector::ShortDialogDetector(VideoOverlay& overlay)
    : m_bottom(overlay, 0.50, 0.91, 0.29, 0.05)
    , m_left_white(overlay, 0.21, 0.835, 0.008, 0.12)
    , m_left(overlay, 0.18, 0.835, 0.02, 0.12)
    , m_right(overlay, 0.822, 0.835, 0.02, 0.12)
{}

bool ShortDialogDetector::detect(const QImage& screen) const{
    ImageStats left_white = image_stats(extract_box(screen, m_left_white));
    if (!is_white(left_white)){
        return false;
    }
    ImageStats bottom = image_stats(extract_box(screen, m_bottom));
    if (!is_white(bottom)){
        return false;
    }
    ImageStats left = image_stats(extract_box(screen, m_left));
//    cout << left.stddev << endl;
    if (left.stddev.sum() < 50){
        return false;
    }
    ImageStats right = image_stats(extract_box(screen, m_right));
//    cout << right.stddev << endl;
    if (right.stddev.sum() < 50){
        return false;
    }

    return true;
}

ShortDialogDetectorCallback::ShortDialogDetectorCallback(VideoOverlay& overlay)
    : ShortDialogDetector(overlay)
{}
bool ShortDialogDetectorCallback::process_frame(
    const QImage& frame,
    std::chrono::system_clock::time_point
){
    return detect(frame);
}




BattleDialogDetector::BattleDialogDetector(VideoOverlay& overlay)
    : m_bottom(overlay, 0.50, 0.91, 0.40, 0.05)
    , m_left_white(overlay, 0.07, 0.835, 0.008, 0.12)
    , m_left(overlay, 0.04, 0.835, 0.02, 0.12)
    , m_right(overlay, 0.965, 0.835, 0.02, 0.12)
{}

bool BattleDialogDetector::detect(const QImage& screen) const{
    ImageStats left_white = image_stats(extract_box(screen, m_left_white));
    if (!is_white(left_white)){
        return false;
    }
    ImageStats bottom = image_stats(extract_box(screen, m_bottom));
    if (!is_white(bottom)){
        return false;
    }
    ImageStats left = image_stats(extract_box(screen, m_left));
//    cout << left.stddev << endl;
    if (left.stddev.sum() < 50){
        return false;
    }
    ImageStats right = image_stats(extract_box(screen, m_right));
//    cout << right.stddev << endl;
    if (right.stddev.sum() < 50){
        return false;
    }

    return true;
}




}
}
}
