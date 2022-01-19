/*  Receive Pokemon (Orange Background) Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Compiler.h"
#include "CommonFramework/Tools/VideoOverlaySet.h"
#include "CommonFramework/Inference/ImageTools.h"
#include "PokemonSwSh_ReceivePokemonDetector.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


ReceivePokemonDetector::ReceivePokemonDetector()
    : VisualInferenceCallback("ReceivePokemonDetector")
    , m_box_top(0.2, 0.02, 0.78, 0.02)
    , m_box_top_right(0.93, 0.02, 0.05, 0.1)
    , m_box_bot_left(0.02, 0.85, 0.1, 0.1)
    , m_has_been_orange(false)
{}
void ReceivePokemonDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_box_top);
    items.add(COLOR_RED, m_box_top_right);
    items.add(COLOR_RED, m_box_bot_left);
}

bool ReceivePokemonDetector::process_frame(
    const QImage& frame,
    std::chrono::system_clock::time_point timestamp
){
    return receive_is_over(frame);
}
bool ReceivePokemonDetector::receive_is_over(const QImage& frame){
    QImage image0 = extract_box(frame, m_box_top);
    QImage image1 = extract_box(frame, m_box_top_right);
    QImage image2 = extract_box(frame, m_box_bot_left);

    ImageStats stats0 = image_stats(image0);
    ImageStats stats1 = image_stats(image1);
    ImageStats stats2 = image_stats(image2);

    FloatPixel expected(193, 78, 56);
    FloatPixel actual0 = stats0.average;
    FloatPixel actual1 = stats1.average;
    FloatPixel actual2 = stats2.average;

    expected /= expected.sum();
    actual0 /= actual0.sum();
    actual1 /= actual1.sum();
    actual2 /= actual2.sum();

    double distance0 = euclidean_distance(expected, actual0);
    double distance1 = euclidean_distance(expected, actual1);
    double distance2 = euclidean_distance(expected, actual2);

    if (euclidean_distance(actual0, actual1) > 10){
        return m_has_been_orange;
    }
    if (euclidean_distance(actual0, actual2) > 10){
        return m_has_been_orange;
    }
    if (euclidean_distance(actual1, actual2) > 10){
        return m_has_been_orange;
    }
    if (stats0.stddev.sum() > 10 || distance0 > 0.2){
        return m_has_been_orange;
    }
    if (stats1.stddev.sum() > 10 || distance1 > 0.2){
        return m_has_been_orange;
    }
    if (stats2.stddev.sum() > 10 || distance2 > 0.2){
        return m_has_been_orange;
    }

    m_has_been_orange = true;
    return false;
}


}
}
}
