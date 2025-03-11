/*  Flag Tracker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cmath>
#include <map>
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "PokemonLA_FlagTracker.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{



FlagTracker::FlagTracker(Logger& /*logger*/, VideoOverlay& overlay)
    : VisualInferenceCallback("FlagTracker")
    // , m_logger(logger)
    , m_watcher(overlay, {0, 0, 1, 1}, {{m_flags, false}})
{}

void FlagTracker::make_overlays(VideoOverlaySet& items) const{
    m_watcher.make_overlays(items);
}

bool FlagTracker::get(
    double& distance, double& x, double& y,
    WallClock timestamp
) const{
    ReadSpinLock lg(m_lock);

    //  If history is empty or stale, return no detection.
    if (m_history.empty() || m_history.back().timestamp + std::chrono::milliseconds(500) < timestamp){
        return false;
    }

    {
        const Sample& sample = m_history.back();
        x = sample.x;
        y = sample.y;
    }

    //  Distance reading is unreliable. So look at the last 2 seconds of history to infer it.
    std::multimap<int, WallClock> distances;
    for (const Sample& sample : m_history){
        if (0 <= sample.distance && sample.distance <= 999){
            distances.emplace(sample.distance, sample.timestamp);
        }
    }

    if (distances.size() < 5){
        distance = -1;
        return true;
    }

    //  Find the median.
    double median;
    {
        size_t mid = distances.size() / 2;
        auto iter = distances.begin();
        for (size_t c = 0; c < mid; c++){
            ++iter;
        }
        median = iter->first;
    }

//    distance = median;
//    cout << distance << endl;

    //  Pick the latest value that isn't too far from the median.
    for (auto iter = m_history.rbegin(); iter != m_history.rend(); ++iter){
        if (std::abs(iter->distance - median) < 20){
            distance = iter->distance;
            return true;
        }
    }
    distance = -1;
    return true;
}

bool FlagTracker::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    m_watcher.process_frame(frame, timestamp);

    Sample sample;

    const std::vector<ImagePixelBox>& flags = m_flags.detections();
    bool ok = flags.size() == 1;
    if (ok){
        sample.timestamp = timestamp;
        sample.x = (double)(flags[0].min_x + flags[0].max_x) / (frame.width() * 2);
        sample.y = (double)(flags[0].min_y + flags[0].max_y) / (frame.height() * 2);
        sample.distance = read_flag_distance(frame, sample.x, sample.y);

#if 0
//        cout << sample.distance << endl;
        if (sample.distance > 0 && sample.distance < 10){
            static int c = 0;
            frame.save("test-" + std::to_string(c++) + ".png");
        }
#endif
    }else{
//        frame.save("test.png");
//        cout << "no flag" << endl;
    }


    WriteSpinLock lg(m_lock);

    //  Clear out old history.
    WallClock threshold = timestamp - std::chrono::seconds(2);
    while (!m_history.empty() && m_history.front().timestamp < threshold){
        m_history.pop_front();
    }

    if (ok){
        m_history.emplace_back(std::move(sample));
    }

    return false;
}





}
}
}
