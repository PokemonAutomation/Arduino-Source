/*  Dialog Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QImage>
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "CommonFramework/Tools/VideoOverlaySet.h"
#include "PokemonLA_CommonColorCheck.h"
#include "PokemonLA_DialogDetector.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


DialogSurpriseDetector::DialogSurpriseDetector(LoggerQt& logger, VideoOverlay& overlay, bool stop_on_detected)
    : VisualInferenceCallback("DialogDetector")
    , m_stop_on_detected(stop_on_detected)
    , m_detected(false)
    , m_title_top   (0.295, 0.722, 0.100, 0.005)
    , m_title_bottom(0.295, 0.765, 0.100, 0.005)
    , m_top_white   (0.500, 0.760, 0.200, 0.020)
    , m_bottom_white(0.400, 0.900, 0.200, 0.020)
    , m_cursor      (0.720, 0.855, 0.030, 0.060)
    , m_arc_phone(logger, overlay, std::chrono::milliseconds(0), false)
{}
void DialogSurpriseDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_title_top);
    items.add(COLOR_RED, m_title_bottom);
    items.add(COLOR_RED, m_top_white);
    items.add(COLOR_RED, m_bottom_white);
    items.add(COLOR_RED, m_cursor);
    m_arc_phone.make_overlays(items);
}
bool DialogSurpriseDetector::process_frame(
    const QImage& frame,
    std::chrono::system_clock::time_point timestamp
){
    size_t hits = 0;

    ImageStats title_top = image_stats(extract_box_reference(frame, m_title_top));
//    cout << title_top.average << title_top.stddev << endl;
//    hits += is_solid(title_top, {0.218332, 0.330301, 0.451367}, 0.2, 15) ? 1 : 0;
    if (is_LA_dark_blue(title_top)){
        hits++;
    }
//    cout << "hits = " << hits << endl;

    ImageStats title_bottom = image_stats(extract_box_reference(frame, m_title_bottom));
//    cout << title_bottom.average << title_bottom.stddev << endl;
//    hits += is_solid(title_bottom, {0.226944, 0.323437, 0.449619}, 0.2, 15) ? 1 : 0;
    if (is_LA_dark_blue(title_bottom)){
        hits++;
    }
//    cout << "hits = " << hits << endl;

    ImageStats top_white = image_stats(extract_box_reference(frame, m_top_white));
//    cout << top_white.average << top_white.stddev << endl;
    hits += is_white(top_white, 480, 20) ? 1 : 0;
//    cout << "hits = " << hits << endl;

    ImageStats bottom_white = image_stats(extract_box_reference(frame, m_bottom_white));
//    cout << bottom_white.average << bottom_white.stddev << endl;
    hits += is_white(bottom_white, 480, 20) ? 1 : 0;
//    cout << "hits = " << hits << endl;

    ImageStats cursor = image_stats(extract_box_reference(frame, m_cursor));
//    cout << cursor.average << cursor.stddev << endl;
    hits += cursor.stddev.sum() > 50 ? 1 : 0;
//    cout << "hits = " << hits << endl;

    m_arc_phone.process_frame(frame, timestamp);
    bool phone = m_arc_phone.detected();
//    cout << !phone << endl;
    hits += !phone;
//    cout << "hits = " << hits << endl;

    bool detected = hits >= 5;
    m_detected.store(detected, std::memory_order_release);

#if 0
    if (detected){
        static size_t c = 0;
        frame.save("SurpriseDialogueBoxTriggered-" + QString::number(c++) + ".png");
    }
#endif


    return detected && m_stop_on_detected;
}






NormalDialogDetector::NormalDialogDetector(LoggerQt& logger, VideoOverlay& overlay, bool stop_on_detected)
    : VisualInferenceCallback("DialogDetector")
    , m_stop_on_detected(stop_on_detected)
    , m_detected(false)
    , m_title_top   (0.278, 0.712, 0.100, 0.005)
    , m_title_bottom(0.278, 0.755, 0.100, 0.005)
    , m_title_left  (0.259, 0.715, 0.003, 0.043)
    , m_title_right (0.390, 0.715, 0.003, 0.043)
    , m_top_white   (0.500, 0.750, 0.200, 0.020)
    , m_bottom_white(0.400, 0.895, 0.200, 0.020)
    , m_left_white  (0.230, 0.805, 0.016, 0.057)
    , m_right_white (0.755, 0.805, 0.016, 0.057)
    // , m_arc_phone(logger, overlay, std::chrono::milliseconds(0), false)
{}
void NormalDialogDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_title_top);
    items.add(COLOR_RED, m_title_bottom);
    items.add(COLOR_RED, m_top_white);
    items.add(COLOR_RED, m_bottom_white);
    items.add(COLOR_RED, m_left_white);
    items.add(COLOR_RED, m_right_white);
    // m_arc_phone.make_overlays(items);
}
bool NormalDialogDetector::process_frame(
    const QImage& frame,
    std::chrono::system_clock::time_point timestamp
){
    size_t hits = 0;

    const ImageStats title_top = image_stats(extract_box_reference(frame, m_title_top));
    const ImageStats title_bottom = image_stats(extract_box_reference(frame, m_title_bottom));
    const ImageStats title_left = image_stats(extract_box_reference(frame, m_title_left));
    const ImageStats title_right = image_stats(extract_box_reference(frame, m_title_right));
    // If three of the four passes, then we are good for title check
    if (is_LA_dark_blue(title_top) + is_LA_dark_blue(title_bottom) + is_LA_dark_blue(title_left) + is_LA_dark_blue(title_right) >= 3){
        hits++;
    }

    ImageStats top_white = image_stats(extract_box_reference(frame, m_top_white));
    hits += is_white(top_white, 480, 30) ? 1 : 0;

    ImageStats bottom_white = image_stats(extract_box_reference(frame, m_bottom_white));
    hits += is_white(bottom_white, 480, 30) ? 1 : 0;

    ImageStats left_white = image_stats(extract_box_reference(frame, m_left_white));
    hits += is_white(left_white, 480, 30) ? 1 : 0;

    ImageStats right_white = image_stats(extract_box_reference(frame, m_right_white));
    hits += is_white(right_white, 480, 30) ? 1 : 0;

    // m_arc_phone.process_frame(frame, timestamp);
    // bool phone = m_arc_phone.detected();
    // hits += !phone;

    bool detected = hits == 5;
    m_detected.store(detected, std::memory_order_release);

    return detected && m_stop_on_detected;
}

}
}
}
