/*  Dialogue Yellow Arrow Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/ImageTools/BinaryImage_FilterRgb32.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "PokemonLA_DialogueYellowArrowDetector.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

using Kernels::Waterfill::WaterfillObject;


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
    LoggerQt& logger, VideoOverlay& overlay,
    bool stop_on_detected
)
    : VisualInferenceCallback("DialogueYellowArrowDetector")
    , m_logger(logger)
    , m_box(0.720, 0.759, 0.049, 0.128)  // This box covers all possible locations of the yellow arrow
    , m_stop_on_detected(stop_on_detected)
{}


void DialogueYellowArrowDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_box);
}
bool DialogueYellowArrowDetector::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    auto cropped_frame = extract_box_reference(frame, m_box);

    auto matrices = compress_rgb32_to_binary_range(cropped_frame,{
        {combine_rgb(160, 160, 0), combine_rgb(255, 255, 255)},
        {combine_rgb(200, 200, 0), combine_rgb(255, 255, 255)},
        {combine_rgb(200, 200, 0), combine_rgb(255, 255, 180)},
    });

    // int count = 0;
    // for(const auto& matrix : matrices){
    //     ImageRGB32 output = cropped_frame.copy();
    //     Kernels::Waterfill::draw_matrix_on_image(matrix, combine_rgb(255, 0, 0), output, 0, 0);
    //     output.save("test_yellow_arrow_" + std::to_string(count) + ".png");
    //     count++;
    // }

    std::unique_ptr<Kernels::Waterfill::WaterfillSession> session = Kernels::Waterfill::make_WaterfillSession();
    Kernels::Waterfill::WaterfillObject object;

    const size_t arrow_min_area = 200;
    bool detected = false;
    for(auto& matrix : matrices){
        session->set_source(matrix);
        auto finder = session->make_iterator(arrow_min_area);
        const bool keep_object_matrix = false;
        while (finder->find_next(object, keep_object_matrix)){
            double rmsd = DialogueYellowArrowMatcher::instance().rmsd_original(cropped_frame, object);
            // cout << "rmsd " << rmsd << " area " << object.area << endl;
            if (rmsd < 80){
                detected = true;
                break;
            }
        }

        if (detected){
            break;
        }
    }

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
