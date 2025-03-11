/*  Dialogue Yellow Arrow Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <stdint.h>
#include <vector>
#include "Common/Cpp/AbstractLogger.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "PokemonLA_DialogueYellowArrowDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

namespace{
    // This box covers all possible locations of the yellow arrow
    ImageFloatBox YELLOW_ARROW_BOX{0.720, 0.759, 0.049, 0.128};
}

class DialogueYellowArrowMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    DialogueYellowArrowMatcher();
    static const DialogueYellowArrowMatcher& instance();
};


DialogueYellowArrowMatcher::DialogueYellowArrowMatcher()
    : WaterfillTemplateMatcher(
        "PokemonLA/YellowArrow-Template.png",
        Color(0xff808008), Color(0xffffffff), 200
    )
{
    m_aspect_ratio_lower = 0.9;
    m_aspect_ratio_upper = 1.1;
    m_area_ratio_lower = 0.9;
    m_area_ratio_upper = 1.1;
}

const DialogueYellowArrowMatcher& DialogueYellowArrowMatcher::instance(){
    static DialogueYellowArrowMatcher matcher;
    return matcher;
}



DialogueYellowArrowDetector::DialogueYellowArrowDetector(
    Logger& logger, VideoOverlay& overlay,
    bool stop_on_detected
)
    : VisualInferenceCallback("DialogueYellowArrowDetector")
    , m_logger(logger)
    , m_stop_on_detected(stop_on_detected)
{}


void DialogueYellowArrowDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, YELLOW_ARROW_BOX);
}
bool DialogueYellowArrowDetector::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    const std::vector<std::pair<uint32_t, uint32_t>> filters = {
        {combine_rgb(160, 160, 0), combine_rgb(255, 255, 255)},
        {combine_rgb(200, 200, 0), combine_rgb(255, 255, 255)},
        {combine_rgb(200, 200, 0), combine_rgb(255, 255, 180)},
    };

    // We found 200 to be a good minimal yellow arrow pixel count on a 1920x1080 resolution screenshot.
    const double screen_scale = frame.height() / 1080.0;
    const size_t min_size = size_t(200 * screen_scale * screen_scale);
    
    const bool detected = match_template_by_waterfill(
        extract_box_reference(frame, YELLOW_ARROW_BOX), 
        DialogueYellowArrowMatcher::instance(),
        filters,
        {min_size, SIZE_MAX},
        80,
        [](Kernels::Waterfill::WaterfillObject& object) -> bool { return true; }
    );

    if (detected){
        m_logger.log("Detected yellow arrow in transparent dialogue box.", COLOR_PURPLE);
    }

    m_detected.store(detected, std::memory_order_release);

#if 0
    if (detected){
        static size_t c = 0;
        frame.save("YellowArrowTriggered-" + std::to_string(c++) + ".png");
    }
#endif

    return detected && m_stop_on_detected;
}





}
}
}
