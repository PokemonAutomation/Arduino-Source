/*  Alert Eye Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "PokemonLZA_AlertEyeDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{



class AlertEyeMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    AlertEyeMatcher()
        : WaterfillTemplateMatcher(
            "PokemonLZA/AlertEye-Template.png",
            Color(200, 100, 100), Color(255, 255, 255), 100
        )
    {
        m_aspect_ratio_lower = 0.9;
        m_aspect_ratio_upper = 1.1;
        m_area_ratio_lower = 0.85;
        m_area_ratio_upper = 1.1;
    }

    static const AlertEyeMatcher& instance(){
        static AlertEyeMatcher matcher;
        return matcher;
    }
};



AlertEyeDetector::AlertEyeDetector(
    Color color,
    VideoOverlay* overlay
)
    : m_color(color)
    , m_overlay(overlay)
    , m_alert_eye_box(0.485, 0.088, 0.029, 0.034)
{}

void AlertEyeDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_alert_eye_box);
}

bool AlertEyeDetector::detect(const ImageViewRGB32& screen){
    const double screen_rel_size = (screen.height() / 1080.0);
    const double screen_rel_size_2 = screen_rel_size * screen_rel_size;

    const double min_area_1080p = 300;
    const double rmsd_threshold = 80;
    const size_t min_area = size_t(screen_rel_size_2 * min_area_1080p);

    const std::vector<std::pair<uint32_t, uint32_t>> FILTERS = {
        { 0xff808080, 0xffffffff },
        { 0xff909090, 0xffffffff },
        { 0xffa0a0a0, 0xffffffff },
        { 0xffb0b0b0, 0xffffffff },
        { 0xffc0c0c0, 0xffffffff },
        { 0xffd0d0d0, 0xffffffff },
        { 0xffe0e0e0, 0xffffffff },
        { 0xfff0f0f0, 0xffffffff },
    };

    bool found = match_template_by_waterfill(
        screen.size(),
        extract_box_reference(screen, m_alert_eye_box),
        AlertEyeMatcher::instance(),
        FILTERS,
        {min_area, SIZE_MAX},
        rmsd_threshold,
        [&](Kernels::Waterfill::WaterfillObject& object) -> bool {
            m_last_detected = translate_to_parent(screen, m_alert_eye_box, object);
            return true;
        }
    );

    if (m_overlay){
        if (found){
            m_last_detected_box.emplace(*m_overlay, m_last_detected, COLOR_GREEN);
        }else{
            m_last_detected_box.reset();
        }
    }

    return found;
}




AlertEyeTracker::AlertEyeTracker(Color color, VideoOverlay* overlay, WallDuration min_duration)
    : AlertEyeDetector(color, overlay)
    , VisualInferenceCallback("AlertEyeTracker")
    , m_min_duration(min_duration)
    , m_first_detection(WallClock::max())
{}

bool AlertEyeTracker::currently_active() const{
    ReadSpinLock lg(m_lock);
    return m_first_detection <= current_time() - m_min_duration;
}

void AlertEyeTracker::make_overlays(VideoOverlaySet& items) const{
    AlertEyeDetector::make_overlays(items);
}

bool AlertEyeTracker::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    if (!detect(frame)){
        WriteSpinLock lg(m_lock);
        m_first_detection = WallClock::max();
        return false;
    }

    WriteSpinLock lg(m_lock);
    m_first_detection = std::min(m_first_detection, timestamp);

    return false;
}







}
}
}
