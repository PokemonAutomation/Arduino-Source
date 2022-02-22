/*  Flag Tracker
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QImage>
#include "PokemonLA_FlagTracker.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{



FlagTracker::FlagTracker(Logger& logger, VideoOverlay& overlay)
    : VisualInferenceCallback("FlagTracker")
    , m_logger(logger)
    , m_watcher(overlay, {0, 0, 1, 1}, {{m_flags, false}})
{}

void FlagTracker::make_overlays(VideoOverlaySet& items) const{
    m_watcher.make_overlays(items);
}

bool FlagTracker::get(double& distance, double& x, double& y){
    SpinLockGuard lg(m_lock);
    if (m_history.empty()){
        return false;
    }
    const Sample& sample = m_history.back();
    distance = sample.distance;
    x = sample.x;
    y = sample.y;
    return true;
}

bool FlagTracker::process_frame(
    const QImage& frame,
    std::chrono::system_clock::time_point timestamp
){
    m_watcher.process_frame(frame, timestamp);

    SpinLockGuard lg(m_lock);

    //  Clear out old history.
    std::chrono::system_clock::time_point threshold = timestamp - std::chrono::seconds(1);
    while (!m_history.empty() && m_history.front().timestamp < threshold){
        m_history.pop_front();
    }

    const std::vector<ImagePixelBox>& flags = m_flags.detections();
//    cout << "flags.size() = " << flags.size() << endl;
    if (flags.size() != 1){
        return false;
    }
    double x_axis = (double)(flags[0].min_x + flags[0].max_x) / (frame.width() * 2);
    double y_axis = (double)(flags[0].min_y + flags[0].max_y) / (frame.width() * 2);

    m_history.emplace_back(Sample{timestamp, -1, x_axis, y_axis});
//    cout << "m_history.size() = " << m_history.size() << endl;

    return false;
}





}
}
}
