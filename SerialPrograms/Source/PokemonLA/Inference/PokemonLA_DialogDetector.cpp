/*  Dialog Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QImage>
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "CommonFramework/Tools/VideoOverlaySet.h"
#include "PokemonLA_DialogDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


DialogDetector::DialogDetector()
    : VisualInferenceCallback("DialogDetector")
    , m_title_top   (0.295, 0.725, 0.100, 0.005)
    , m_title_bottom(0.295, 0.765, 0.100, 0.005)
    , m_top_white   (0.500, 0.760, 0.200, 0.020)
    , m_bottom_white(0.500, 0.900, 0.200, 0.020)
    , m_cursor      (0.720, 0.855, 0.030, 0.060)
{}
void DialogDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_title_top);
    items.add(COLOR_RED, m_title_bottom);
    items.add(COLOR_RED, m_top_white);
    items.add(COLOR_RED, m_bottom_white);
    items.add(COLOR_RED, m_cursor);
}
bool DialogDetector::process_frame(
    const QImage& frame,
    std::chrono::system_clock::time_point timestamp
){
    ImageStats title_top = image_stats(extract_box(frame, m_title_top));
//    cout << title_top.average << title_top.stddev << endl;
    if (!is_solid(title_top, {0.234899, 0.33557, 0.42953})){
        return false;
    }

    ImageStats title_bottom = image_stats(extract_box(frame, m_title_bottom));
//    cout << title_bottom.average << title_bottom.stddev << endl;
    if (!is_solid(title_bottom, {0.234899, 0.33557, 0.42953})){
        return false;
    }

    ImageStats top_white = image_stats(extract_box(frame, m_top_white));
//    cout << top_white.average << top_white.stddev << endl;
    if (!is_solid(top_white, {0.344919, 0.350944, 0.304137})){
        return false;
    }

    ImageStats bottom_white = image_stats(extract_box(frame, m_bottom_white));
//    cout << bottom_white.average << bottom_white.stddev << endl;
    if (!is_solid(bottom_white, {0.344919, 0.350944, 0.304137})){
        return false;
    }

    ImageStats cursor = image_stats(extract_box(frame, m_cursor));
//    cout << cursor.average << cursor.stddev << endl;
    if (cursor.stddev.sum() < 50){
        return false;
    }

    return true;
}



}
}
}
