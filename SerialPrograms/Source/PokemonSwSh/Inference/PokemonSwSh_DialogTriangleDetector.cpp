/*  Dialog Triangle Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <stdint.h>
#include "Common/Cpp/Color.h"
#include "Common/Cpp/AbstractLogger.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "PokemonSwSh_DialogTriangleDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


namespace{
    // This box covers all possible locations of the black triangle arrow
    ImageFloatBox BLACK_TRIANGLE_BOX{0.771, 0.901, 0.031, 0.069};
}

class DialogTriangleMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    DialogTriangleMatcher();
    static const DialogTriangleMatcher& instance();
};


DialogTriangleMatcher::DialogTriangleMatcher()
    : WaterfillTemplateMatcher(
        "PokemonSwSh/DialogBlackTriangle.png",
        Color(0,0,0), Color(30, 30, 30), 50
    )
{
    m_aspect_ratio_lower = 0.9;
    m_aspect_ratio_upper = 1.1;
    m_area_ratio_lower = 0.9;
    m_area_ratio_upper = 1.1;
}

const DialogTriangleMatcher& DialogTriangleMatcher::instance(){
    static DialogTriangleMatcher matcher;
    return matcher;
}



DialogTriangleDetector::DialogTriangleDetector(
    Logger& logger, VideoOverlay& overlay,
    bool stop_on_detected
)
    : VisualInferenceCallback("DialogTriangleDetector")
    , m_logger(logger)
    , m_stop_on_detected(stop_on_detected)
{}


void DialogTriangleDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, BLACK_TRIANGLE_BOX);
}
bool DialogTriangleDetector::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    const std::vector<std::pair<uint32_t, uint32_t>> filters = {
        {combine_rgb(0, 0, 0), combine_rgb(50, 50, 50)}
    };
    
    const double screen_rel_size = (frame.height() / 1080.0);
    const size_t min_size = size_t(screen_rel_size * screen_rel_size * 500.0);

    const bool detected = match_template_by_waterfill(
        extract_box_reference(frame, BLACK_TRIANGLE_BOX), 
        DialogTriangleMatcher::instance(),
        filters,
        {min_size, SIZE_MAX},
        80,
        [](Kernels::Waterfill::WaterfillObject& object) -> bool { return true; }
    );

    if (detected){
        m_logger.log("Detected dialog black triangle.", COLOR_PURPLE);
    }

    m_detected.store(detected, std::memory_order_release);

#if 0
    if (detected){
        static size_t c = 0;
        frame.save("DialogTriangleDetectorTriggered-" + std::to_string(c++) + ".png");
    }
#endif

    return detected && m_stop_on_detected;
}





}
}
}

