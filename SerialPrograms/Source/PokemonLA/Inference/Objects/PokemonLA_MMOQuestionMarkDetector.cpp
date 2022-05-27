/*  MMO Question Marks Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/ImageTools/BinaryImage_FilterRgb32.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Utilities.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "PokemonLA_MMOQuestionMarkDetector.h"
#include "PokemonLA/PokemonLA_Locations.h"

#include <sstream>
#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

using namespace Kernels::Waterfill;

namespace {

// The boxes that cover the locations on the Hisui map that MMO question marks will appear.
const std::array<ImageFloatBox, 5> hisui_map_boxes{{
    {0.362, 0.670, 0.045, 0.075},
    {0.683, 0.555, 0.039, 0.076},
    {0.828, 0.372, 0.042, 0.082},
    {0.485, 0.440, 0.044, 0.080},
    {0.393, 0.144, 0.050, 0.084}
}};
}

MMOQuestionMarkMatcher::MMOQuestionMarkMatcher()
    : WaterfillTemplateMatcher(
        "PokemonLA/MMOQuestionMark-Template.png",
        Color(0xff808080), Color(0xffffffff), 200
    )
{
    m_aspect_ratio_lower = 0.8;
    m_aspect_ratio_upper = 1.2;
    m_area_ratio_lower = 0.8;
    m_area_ratio_upper = 1.2;
}

const MMOQuestionMarkMatcher& MMOQuestionMarkMatcher::instance(){
    static MMOQuestionMarkMatcher matcher;
    return matcher;
}


bool detect_MMO_question_mark(const PokemonAutomation::ConstImageRef &image){
    auto matrix = compress_rgb32_to_binary_multirange(image, {
        // {combine_rgb(0, 20, 40), combine_rgb(60, 70, 100)},
        {combine_rgb(180, 180, 180), combine_rgb(255, 255, 255)},
    });

    // static int count = 0;
    // QImage output = image.to_qimage();
    // draw_matrix_on_image(matrix, combine_rgb(255, 0, 0), output, 0, 0);

    std::unique_ptr<Kernels::Waterfill::WaterfillSession> session = Kernels::Waterfill::make_WaterfillSession();
    Kernels::Waterfill::WaterfillObject object;

    // The MMO question mark is about 310 pixels on an 1080P image.
    // It should be about 130 pixels on 720P image

    const size_t min_area = 100;
    bool detected = false;
    session->set_source(matrix);
    auto finder = session->make_iterator(min_area);
    const bool keep_object_matrix = true;
    while (finder->find_next(object, keep_object_matrix)){
        if (object.area > 500){
            continue;
        }
        double rmsd = MMOQuestionMarkMatcher::instance().rmsd_original(image, object);
        // cout << "rmsd " << rmsd << " " << object.area << endl;
        if (rmsd < 100){
            // draw_object_on_image(object, combine_rgb(0, 255, 0), output, 0, 0);
            detected = true;
            break;
        }
    }

    // output.save(QString::fromStdString("test_mmo_question_marks" + std::to_string(count) + ".png"));
    // count++;

    return detected;
}



MMOQuestionMarkDetector::MMOQuestionMarkDetector(LoggerQt& logger)
    : m_logger(logger)
{}


void MMOQuestionMarkDetector::make_overlays(VideoOverlaySet& items) const{
    for(size_t i = 0; i < hisui_map_boxes.size(); i++){
        items.add(COLOR_RED, hisui_map_boxes[i]);
    }
}

std::array<bool, 5> MMOQuestionMarkDetector::detect_MMO_on_hisui_map(const QImage& frame){
    std::array<bool, 5> detected{false};
    for(size_t i = 0; i < hisui_map_boxes.size(); i++){
        auto cropped_frame = extract_box_reference(frame, hisui_map_boxes[i]);
        detected[i] = detect_MMO_question_mark(cropped_frame);
    }
    
    if (std::find(detected.begin(), detected.end(), true) != detected.end()){
        std::ostringstream os;
        os << "Detected MMO question mark on region ";
        for(size_t i = 0; i < detected.size(); i++){
            if (detected[i]){
                os << WILD_REGION_SHORT_NAMES[i] << ", ";
            }
        }
        m_logger.log(os.str(), COLOR_PURPLE);
    }

    return detected;
}

void add_MMO_detection_to_overlay(const std::array<bool, 5>& detection_result, VideoOverlaySet& items){
    for(size_t i = 0; i < hisui_map_boxes.size(); i++){
        if (detection_result[i]){
            items.add(COLOR_CYAN, hisui_map_boxes[i]);
        }
    }
}



}
}
}
