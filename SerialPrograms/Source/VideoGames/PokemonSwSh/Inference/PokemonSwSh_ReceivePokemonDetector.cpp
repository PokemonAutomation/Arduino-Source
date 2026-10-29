/*  Receive Pokemon (Orange Background) Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "PokemonSwSh_ReceivePokemonDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


ReceivePokemonDetector::ReceivePokemonDetector(Color color)
    : m_color(color)
    , m_box_top(0.2, 0.02, 0.78, 0.02)
    , m_box_top_right(0.93, 0.02, 0.05, 0.1)
    , m_box_bot_left(0.02, 0.85, 0.1, 0.1)
{}
void ReceivePokemonDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_box_top);
    items.add(COLOR_RED, m_box_top_right);
    items.add(COLOR_RED, m_box_bot_left);
}
bool ReceivePokemonDetector::detect(const ImageViewRGB32& screen){
    ImageStats stats0 = image_stats(extract_box_reference(screen, m_box_top));
    ImageStats stats1 = image_stats(extract_box_reference(screen, m_box_top_right));
    ImageStats stats2 = image_stats(extract_box_reference(screen, m_box_bot_left));

    FloatPixel expected(193, 78, 56);
    FloatPixel actual0 = stats0.average;
    FloatPixel actual1 = stats1.average;
    FloatPixel actual2 = stats2.average;

//    cout << actual0 << actual1 << actual2 << endl;

    if (actual0.sum() < 100){
        return false;
    }
    if (actual1.sum() < 100){
        return false;
    }
    if (actual2.sum() < 100){
        return false;
    }

    expected /= expected.sum();
    actual0 /= actual0.sum();
    actual1 /= actual1.sum();
    actual2 /= actual2.sum();

    double distance0 = euclidean_distance(expected, actual0);
    double distance1 = euclidean_distance(expected, actual1);
    double distance2 = euclidean_distance(expected, actual2);

    if (euclidean_distance(actual0, actual1) > 10){
        return false;
    }
    if (euclidean_distance(actual0, actual2) > 10){
        return false;
    }
    if (euclidean_distance(actual1, actual2) > 10){
        return false;
    }
    if (stats0.stddev.sum() > 10 || distance0 > 0.2){
        return false;
    }
    if (stats1.stddev.sum() > 10 || distance1 > 0.2){
        return false;
    }
    if (stats2.stddev.sum() > 10 || distance2 > 0.2){
        return false;
    }

    return true;
}




ReceivePokemonOverWatcher::ReceivePokemonOverWatcher(bool stop_on_detected)
    : VisualInferenceCallback("ReceivePokemonDetector")
    , m_stop_on_detected(stop_on_detected)
    , m_box_top(0.2, 0.02, 0.78, 0.02)
    , m_box_top_right(0.93, 0.02, 0.05, 0.1)
    , m_box_bot_left(0.02, 0.85, 0.1, 0.1)
    , m_has_been_orange(false)
    , m_triggered(false)
{}
void ReceivePokemonOverWatcher::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_box_top);
    items.add(COLOR_RED, m_box_top_right);
    items.add(COLOR_RED, m_box_bot_left);
}

bool ReceivePokemonOverWatcher::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    bool ret = receive_is_over(frame);
    bool triggered = m_triggered.load(std::memory_order_acquire);
    m_triggered.store(triggered | ret, std::memory_order_release);
//    cout << "m_has_been_orange = " << m_has_been_orange << endl;
    return ret && m_stop_on_detected;
}
bool ReceivePokemonOverWatcher::receive_is_over(const ImageViewRGB32& frame){
    ImageStats stats0 = image_stats(extract_box_reference(frame, m_box_top));
    ImageStats stats1 = image_stats(extract_box_reference(frame, m_box_top_right));
    ImageStats stats2 = image_stats(extract_box_reference(frame, m_box_bot_left));

    FloatPixel expected(193, 78, 56);
    FloatPixel actual0 = stats0.average;
    FloatPixel actual1 = stats1.average;
    FloatPixel actual2 = stats2.average;

//    cout << actual0 << actual1 << actual2 << endl;

    if (actual0.sum() < 100){
        return m_has_been_orange;
    }
    if (actual1.sum() < 100){
        return m_has_been_orange;
    }
    if (actual2.sum() < 100){
        return m_has_been_orange;
    }

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
