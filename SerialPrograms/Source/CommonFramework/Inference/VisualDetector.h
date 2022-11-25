/*  Visual Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_VisualDetector_H
#define PokemonAutomation_CommonFramework_VisualDetector_H

#include "CommonFramework/InferenceInfra/VisualInferenceCallback.h"

namespace PokemonAutomation{

class ImageViewRGB32;
class VideoOverlaySet;


class StaticScreenDetector{
public:
    virtual void make_overlays(VideoOverlaySet& items) const = 0;
    virtual bool detect(const ImageViewRGB32& screen) const = 0;
};



//  Wrap a detector into a finder.
//  This one requires the detector to return true consecutively for X time
//  before returning true.
template <typename Detector>
class DetectorToFinder : public Detector, public VisualInferenceCallback{
public:
    template <class... Args>
    DetectorToFinder(
        std::string label,
        std::chrono::milliseconds duration,
        Args&&... args
    )
        : Detector(std::forward<Args>(args)...)
        , VisualInferenceCallback(std::move(label))
        , m_duration(duration)
    {}

    virtual void make_overlays(VideoOverlaySet& items) const override{
        Detector::make_overlays(items);
    }
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override{
        if (!this->detect(frame)){
            m_start_of_detection = WallClock::min();
            return false;
        }
        if (m_start_of_detection == WallClock::min()){
            m_start_of_detection = timestamp;
        }

        return timestamp - m_start_of_detection >= m_duration;
    }

private:
    std::chrono::milliseconds m_duration;
    WallClock m_start_of_detection = WallClock::min();
};




}
#endif
