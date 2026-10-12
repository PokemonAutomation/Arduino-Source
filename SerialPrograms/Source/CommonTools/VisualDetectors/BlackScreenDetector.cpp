/*  Black Border Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *
 *      Returns true after a black screen has been detected and
 * the black screen has ended.
 *
 */

#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "BlackScreenDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


BlackScreenDetector::BlackScreenDetector(
    Color color, const ImageFloatBox& box,
    double max_rgb_sum,
    double max_stddev_sum
)
    : m_color(color)
    , m_box(box)
    , m_max_rgb_sum(max_rgb_sum)
    , m_max_stddev_sum(max_stddev_sum)
{}
void BlackScreenDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}
bool BlackScreenDetector::detect(const ImageViewRGB32& screen){
    return is_black(extract_box_reference(screen, m_box), m_max_rgb_sum, m_max_stddev_sum);
}



WhiteScreenDetector::WhiteScreenDetector(
    Color color, const ImageFloatBox& box,
    double min_rgb_sum,
    double max_stddev_sum
)
    : m_color(color)
    , m_box(box)
    , m_min_rgb_sum(min_rgb_sum)
    , m_max_stddev_sum(max_stddev_sum)
{}
void WhiteScreenDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}
bool WhiteScreenDetector::detect(const ImageViewRGB32& screen){
    return is_white(extract_box_reference(screen, m_box), m_min_rgb_sum, m_max_stddev_sum);
}






BlackScreenOverWatcher::BlackScreenOverWatcher(
    Color color, const ImageFloatBox& box,
    double max_rgb_sum,
    double max_stddev_sum,
    std::chrono::milliseconds hold_duration,
    std::chrono::milliseconds release_duration
)
    : VisualInferenceCallback("BlackScreenOverWatcher")
    , m_on(color, box, max_rgb_sum, max_stddev_sum, BlackScreenWatcher::FinderType::PRESENT, hold_duration)
    , m_off(color, box, max_rgb_sum, max_stddev_sum, BlackScreenWatcher::FinderType::GONE, release_duration)
{}
void BlackScreenOverWatcher::make_overlays(VideoOverlaySet& items) const{
    m_on.make_overlays(items);
}
bool BlackScreenOverWatcher::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    if (m_black_is_over.load(std::memory_order_acquire)){
        return true;
    }
    if (!m_has_been_black){
        m_has_been_black = m_on.process_frame(frame, timestamp);
//        cout << "m_has_been_black = " << m_has_been_black << endl;
        return false;
    }

    bool is_over = m_off.process_frame(frame, timestamp);
    if (!is_over){
        return false;
    }
    m_black_is_over.store(true, std::memory_order_release);
    return true;
}
bool BlackScreenOverWatcher::black_is_over(const ImageViewRGB32& frame){
    return m_black_is_over.load(std::memory_order_acquire);
}




WhiteScreenOverWatcher::WhiteScreenOverWatcher(
    Color color, const ImageFloatBox& box,
    double min_rgb_sum,
    double max_stddev_sum
)
    : VisualInferenceCallback("BlackScreenOverWatcher")
    , m_detector(color, box, min_rgb_sum, max_stddev_sum)
{}
void WhiteScreenOverWatcher::make_overlays(VideoOverlaySet& items) const{
    m_detector.make_overlays(items);
}

bool WhiteScreenOverWatcher::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    return white_is_over(frame);
}
bool WhiteScreenOverWatcher::white_is_over(const ImageViewRGB32& frame){
    if (m_detector.detect(frame)){
        m_has_been_white = true;
        return false;
    }
    return m_has_been_white;
}






}
