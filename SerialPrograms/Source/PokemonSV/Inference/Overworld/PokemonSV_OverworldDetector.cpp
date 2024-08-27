/*  Overworld Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/ImageMatch/ImageDiff.h"
#include "CommonFramework/ImageMatch/ExactImageMatcher.h"
#include "CommonFramework/ImageTools/BinaryImage_FilterRgb32.h"
#include "CommonFramework/ImageTools/WaterfillUtilities.h"
#include "CommonFramework/ImageMatch/WaterfillTemplateMatcher.h"
#include "PokemonSV_OverworldDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



const ImageMatch::ExactImageMatcher& RADAR_BALL(){
    static ImageMatch::ExactImageMatcher matcher(RESOURCE_PATH() + "PokemonSV/RadarBall.png");
    return matcher;
}

class RadarBallMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    RadarBallMatcher() : WaterfillTemplateMatcher(
        "PokemonSV/RadarBall.png", Color(192,192,0), Color(255, 255, 127), 5
    ){
        m_aspect_ratio_lower = 0.8;
        m_aspect_ratio_upper = 1.2;
        m_area_ratio_lower = 0.01;
        m_area_ratio_upper = 1.2;
    }

    static const ImageMatch::WaterfillTemplateMatcher& instance(){
        static RadarBallMatcher matcher;
        return matcher;
    }
};



OverworldDetector::OverworldDetector(Color color)
    : m_color(color)
    , m_ball(0.890, 0.800, 0.030, 0.060)
    , m_radar(0.815, 0.680, 0.180, 0.310)
    , m_radar_inside(0.865, 0.750, 0.080, 0.170)
{}
void OverworldDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_ball);
    items.add(m_color, m_radar);
    items.add(m_color, m_radar_inside);
}
bool OverworldDetector::detect(const ImageViewRGB32& screen) const{
    return locate_ball(screen, false).first > 0;
}

std::pair<double, double> OverworldDetector::locate_ball(const ImageViewRGB32& screen, bool strict_requirements) const{
    const std::vector<std::pair<uint32_t, uint32_t>> filters = {
        {0xffc0a000, 0xffffff1f},
        {0xffc0b000, 0xffffff1f},
        {0xffc0c000, 0xffffff1f},
        {0xffd0d000, 0xffffff1f},
        {0xffe0e000, 0xffffff1f},
        {0xfff0f000, 0xffffff1f},
        {0xfff8f800, 0xffffff1f},

        {0xffc0c000, 0xffffff3f},
        {0xffd0d000, 0xffffff3f},
        {0xffe0e000, 0xffffff3f},
        {0xfff0f000, 0xffffff3f},
        {0xfff8f800, 0xffffff3f},

        {0xffc0c000, 0xffffff5f},
        {0xffd0d000, 0xffffff5f},
        {0xffe0e000, 0xffffff5f},
        {0xfff0f000, 0xffffff5f},
        {0xfff8f800, 0xffffff5f},

        {0xffc0c000, 0xffffff7f},
        {0xffd0d000, 0xffffff7f},
        {0xffe0e000, 0xffffff7f},
        {0xfff0f000, 0xffffff7f},
        {0xfff8f800, 0xffffff7f},
    };

    // yellow arrow has area of 70-80. the yellow ball, when only partially filled (i.e. only the outer ring is waterfilled), has an area of 200. 
    // when the ball is fully filled in, it has an area of 550
    const double min_object_size = strict_requirements ? 150.0 : 0;
    const double rmsd_threshold = strict_requirements ? 35.0 : 50.0;

    const double screen_rel_size = (screen.height() / 1080.0);
    const size_t min_size = size_t(screen_rel_size * screen_rel_size * min_object_size);

    std::pair<double, double> ball_location(-1.0, -1.0);
    ImageViewRGB32 cropped = extract_box_reference(screen, m_ball);
    ImagePixelBox pixel_box = floatbox_to_pixelbox(screen.width(), screen.height(), m_ball);
    match_template_by_waterfill(
        cropped, 
        RadarBallMatcher::instance(),
        filters,
        {min_size, SIZE_MAX},
        rmsd_threshold,
        [&](Kernels::Waterfill::WaterfillObject& object) -> bool {
            //  Exclude if it touches the borders of the box
            if (object.min_x == 0 || object.min_y == 0 ||
                object.max_x == cropped.width() || object.max_y == cropped.height()
            ){
                return false;
            }

            ball_location = std::make_pair(
                (object.center_of_gravity_x() + pixel_box.min_x) / (double)screen.width(),
                (object.center_of_gravity_y() + pixel_box.min_y) / (double)screen.height()
            );
            return true;
        }
    );

    return ball_location;
}



OverworldWatcher::OverworldWatcher(Color color, bool detect_event)
     : OverworldDetector(color)
     , VisualInferenceCallback("OverworldWatcher")
     , m_ball_hold_duration(std::chrono::milliseconds(5000))
     , m_map_hold_duration(std::chrono::milliseconds(1000))
     , m_last_ball(WallClock::min())
     , m_detect_event(detect_event)
{}

void OverworldWatcher::make_overlays(VideoOverlaySet& items) const{
    OverworldDetector::make_overlays(items);
}
bool OverworldWatcher::process_frame(const VideoSnapshot& frame){
    //  Return true if either of the following is true:
    //    - Ball is held and radar map stays still for 1 second.
    //    - Ball is held for 5 seconds.

    //  The map is not static when there is an event raid in it as it will
    //  sparkle. So instead, we revert to the ball being held for 5 seconds.

    //  No detection.
    if (!detect(frame)){
        m_last_ball = WallClock::min();
        m_start_of_detection.clear();
        return false;
    }

    //  First detection.
    if (m_last_ball == WallClock::min()){
        m_last_ball = frame.timestamp;
    }
    if (!m_start_of_detection){
        m_start_of_detection = frame;
        return false;
    }

    //  Ball held for long enough.
    if (!m_detect_event && (frame.timestamp - m_last_ball >= m_ball_hold_duration)){
        return true;
    }

    //  Mismatching image sizes.
    ImageViewRGB32 start = extract_box_reference(m_start_of_detection, m_detect_event ? m_ball : m_radar_inside);
    ImageViewRGB32 current = extract_box_reference(frame, m_detect_event ? m_ball : m_radar_inside);
    if (start.width() != current.width() || start.height() != current.height()){
        m_start_of_detection = frame;
        return false;
    }

    //  Image has changed too much.
    double rmsd = ImageMatch::pixel_RMSD(start, current);
//    cout << "rmsd = " << rmsd << endl;
    if (rmsd > 2.0){
        m_start_of_detection = frame;
        return false;
    }

    //  Make sure it's held for long enough.
    return frame.timestamp - m_start_of_detection.timestamp >= m_map_hold_duration;
}





}
}
}
