/*  Visual Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonTools_VisualDetector_H
#define PokemonAutomation_CommonTools_VisualDetector_H

#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"

namespace PokemonAutomation{

class ImageViewRGB32;
class VideoOverlaySet;


class StaticScreenDetector{
public:
    virtual ~StaticScreenDetector() = default;
    virtual void make_overlays(VideoOverlaySet& items) const = 0;

    //  This is not const so that detectors can save/cache state.
    virtual bool detect(const ImageViewRGB32& screen) = 0;

    virtual void commit_state(){}
};



//  Wrap a detector into a finder.
//  This one requires the detector to return the same result consecutively for X time
//  before returning true from process_frame().
template <typename Detector>
class DetectorToFinder : public Detector, public VisualInferenceCallback{
public:
    //  Whether a finder is to find an object is detected by the detector or find
    //  that it's no longer detected.
    enum class FinderType{
        PRESENT,    //  process_frame() returns true only when detected consecutively
        GONE,       //  process_frame() returns true only when not detected consecutively
        CONSISTENT, //  process_frame() returns true when detected consecutively or not detected consecutively
    };

    template <class... Args>
    DetectorToFinder(
        std::string label,
        std::chrono::milliseconds duration,
        Args&&... args
    )
        : Detector(std::forward<Args>(args)...)
        , VisualInferenceCallback(std::move(label))
        , m_duration(duration)
        , m_finder_type(FinderType::PRESENT)
    {}

    
    template <class... Args>
    DetectorToFinder(
        std::string label,
        FinderType finder_type,
        std::chrono::milliseconds duration,
        Args&&... args
    )
        : Detector(std::forward<Args>(args)...)
        , VisualInferenceCallback(std::move(label))
        , m_duration(duration)
        , m_finder_type(finder_type)
    {}

    virtual void make_overlays(VideoOverlaySet& items) const override{
        Detector::make_overlays(items);
    }

    //  If m_finder_type is PRESENT, return true only when it is consecutively detected.
    //  If m_finder_type is GONE, return true only when it is consecutively not detected.
    //  if m_finder_type is CONSISTENT, return true when it is consecutively detected, or consecutively not detected.
    using VisualInferenceCallback::process_frame;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override{
        switch (m_finder_type){
        case FinderType::PRESENT:
        case FinderType::GONE:
            if (this->detect(frame) == (m_finder_type == FinderType::GONE)){
                m_start_of_detection = WallClock::min();
                return false;
            }
            if (m_start_of_detection == WallClock::min()){
                m_start_of_detection = timestamp;
            }

            if (timestamp - m_start_of_detection >= m_duration){
                this->commit_state();
                return true;
            }else{
                return false;
            }
        case FinderType::CONSISTENT:{
            const bool result = this->detect(frame);
            const bool result_changed = (result && m_last_detected < 0) || (!result && m_last_detected > 0);

            m_last_detected = (result ? 1 : -1);

            if (result_changed){
                m_start_of_detection = WallClock::min();
                return false;
            }
            if (m_start_of_detection == WallClock::min()){
                m_start_of_detection = timestamp;
            }

            const bool enough_time = timestamp - m_start_of_detection >= m_duration;
            if (enough_time){
                m_consistent_result = m_last_detected > 0;
            }
            if (enough_time){
                this->commit_state();
                return true;
            }else{
                return false;
            }
        }
        default:;
        }
        return false;
    }

    //  If m_finder_type is CONSISTENT and process_frame() returns true,
    //  whether it is consecutively detected , or consecutively not detected.
    bool consistent_result() const { return m_consistent_result; }

private:
    std::chrono::milliseconds m_duration;
    FinderType m_finder_type;
    WallClock m_start_of_detection = WallClock::min();
    int8_t m_last_detected = 0; // 0: no prior detection, 1: last detected positive, -1: last detected negative
    bool m_consistent_result = false;
};




}
#endif
