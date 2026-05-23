/*  Dialog Triangle Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <stdint.h>
#include <array>
#include <iostream>
using std::cout;
using std::endl;

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "PokemonSwSh_DialogBoxDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


namespace{
    // This box covers all possible locations of the black triangle arrow
    std::array<ImageFloatBox, 4> BLACK_BOXES{{
        {0.254, 0.806, 0.012, 0.052},
        {0.730, 0.806, 0.016, 0.068},
        {0.741, 0.880, 0.005, 0.055},
        {0.710, 0.880, 0.007, 0.055},
    }};
}


BlackDialogBoxDetector::BlackDialogBoxDetector(
    bool stop_on_detected
)
    : VisualInferenceCallback("BlackDialogBoxDetector")
    , m_stop_on_detected(stop_on_detected)
{}


void BlackDialogBoxDetector::make_overlays(VideoOverlaySet& items) const{
    for (const auto& box : BLACK_BOXES){
        items.add(COLOR_RED, box);
    }
}


bool BlackDialogBoxDetector::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    bool detected = true;
    for (const auto& box : BLACK_BOXES){
        if (is_black(extract_box_reference(frame, box), 200, 35) == false){
            detected = false;
            break;
        }
    }

    m_detected.store(detected, std::memory_order_release);

    return detected && m_stop_on_detected;
}




namespace{
    std::array<ImageFloatBox, 2> EGG_HATCH_DIALOG_BOXES{{
        {0.200, 0.900, 0.520, 0.050},
        {0.270, 0.820, 0.480, 0.050},
    }};
}

EggHatchBlackDialogBoxDetector::EggHatchBlackDialogBoxDetector(bool stop_on_detected)
    : VisualInferenceCallback("EggHatchBlackDialogBoxDetector")
    , m_stop_on_detected(stop_on_detected)
    , m_detected(false)
{}

void EggHatchBlackDialogBoxDetector::make_overlays(VideoOverlaySet& items) const{
    for (const auto& box : EGG_HATCH_DIALOG_BOXES){
        items.add(COLOR_YELLOW, box);
    }
}

bool EggHatchBlackDialogBoxDetector::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    static size_t s_frame_count = 0;
    ++s_frame_count;

    bool detected = true;
    for (size_t i = 0; i < EGG_HATCH_DIALOG_BOXES.size(); ++i){
        ImageStats stats = image_stats(extract_box_reference(frame, EGG_HATCH_DIALOG_BOXES[i]));
        // Print once per ~60 frames (~1s). Only runs while open_menu_to_fly is active.
        if (s_frame_count % 60 == 0){
            cout << "EggHatchBlackDialogBox box[" << i << "]: avg_sum=" << stats.average.sum()
                 << " stddev_sum=" << stats.stddev.sum() << endl;
        }
        if (!is_black(stats, 210, 30)){
            detected = false;
            break;
        }
    }
    m_detected.store(detected, std::memory_order_release);
    return detected && m_stop_on_detected;
}


WhiteDialogBoxDetector::WhiteDialogBoxDetector(Color color)
    : m_color(color)
    , m_right(0.782, 0.850, 0.030, 0.050)
    , m_triangle(color)
{}

void WhiteDialogBoxDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_right);
    m_triangle.make_overlays(items);
}
bool WhiteDialogBoxDetector::detect(const ImageViewRGB32& screen){
    ImageStats right = image_stats(extract_box_reference(screen, m_right));
    if (!is_grey(right, 400, 1000)){
        return false;
    }

    if (!m_triangle.detect(screen)){
        return false;
    }

    return true;
}






}
}
}

