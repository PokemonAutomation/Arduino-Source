/*  Dialog Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "CommonFramework/Inference/ImageTools.h"
#include "PokemonBDSP_DialogDetector.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


ShortDialogDetector::ShortDialogDetector(Color color)
    : m_color(color)
//    , m_bottom(0.50, 0.91, 0.29, 0.05)
    , m_left_white(0.21, 0.835, 0.008, 0.12)
    , m_left(0.18, 0.835, 0.02, 0.12)
    , m_right_white(0.785, 0.835, 0.008, 0.12)
    , m_right(0.822, 0.835, 0.02, 0.12)
{}
void ShortDialogDetector::make_overlays(OverlaySet& items) const{
//    items.add(m_color, m_bottom);
    items.add(m_color, m_left_white);
    items.add(m_color, m_left);
    items.add(m_color, m_right_white);
    items.add(m_color, m_right);
}
bool ShortDialogDetector::detect(const QImage& screen) const{
    ImageStats left_white = image_stats(extract_box(screen, m_left_white));
    if (!is_white(left_white)){
        return false;
    }
    ImageStats right_white = image_stats(extract_box(screen, m_right_white));
    if (!is_white(right_white)){
        return false;
    }
//    ImageStats bottom = image_stats(extract_box(screen, m_bottom));
//    if (!is_white(bottom)){
//        return false;
//    }
    ImageStats left = image_stats(extract_box(screen, m_left));
//    cout << left.stddev << endl;
    if (left.stddev.sum() < 30){
        return false;
    }
    ImageStats right = image_stats(extract_box(screen, m_right));
//    cout << right.stddev << endl;
    if (right.stddev.sum() < 30){
        return false;
    }
    return true;
}


void ShortDialogWatcher::make_overlays(OverlaySet& items) const{
    ShortDialogDetector::make_overlays(items);
}
bool ShortDialogWatcher::process_frame(
    const QImage& frame,
    std::chrono::system_clock::time_point
){
    return detect(frame);
}




BattleDialogDetector::BattleDialogDetector(Color color)
    : m_color(color)
    , m_bottom(0.50, 0.91, 0.40, 0.05)
    , m_left_white(0.07, 0.835, 0.008, 0.12)
    , m_left(0.04, 0.835, 0.02, 0.12)
    , m_right(0.965, 0.835, 0.02, 0.12)
{}
void BattleDialogDetector::make_overlays(OverlaySet& items) const{
    items.add(m_color, m_bottom);
    items.add(m_color, m_left_white);
    items.add(m_color, m_left);
    items.add(m_color, m_right);
}
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
