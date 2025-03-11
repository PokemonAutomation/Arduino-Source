/*  Black Out Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "PokemonLA_BlackOutDetector.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

namespace{

// const std::array<ImageSolidCheck, 18> dropped_item_bg_checks = {
//     ImageSolidCheck(0.031, 0.900, 0.024, 0.017, 0.444, 0.440, 0.116, 0.15, 20.0),
//     ImageSolidCheck(0.026, 0.743, 0.021, 0.038, 0.407, 0.397, 0.196, 0.15, 50.0),
//     ImageSolidCheck(0.022, 0.581, 0.020, 0.018, 0.332, 0.433, 0.234, 0.15, 30.0),
//     ImageSolidCheck(0.152, 0.482, 0.017, 0.040, 0.343, 0.451, 0.207, 0.15, 30.0),
//     ImageSolidCheck(0.016, 0.218, 0.018, 0.021, 0.341, 0.350, 0.309, 0.15, 20.0),
//     ImageSolidCheck(0.169, 0.103, 0.024, 0.036, 0.216, 0.385, 0.399, 0.15, 10.0),
//     ImageSolidCheck(0.815, 0.244, 0.026, 0.026, 0.339, 0.350, 0.310, 0.15, 40.0),
//     ImageSolidCheck(0.737, 0.126, 0.045, 0.056, 0.227, 0.384, 0.389, 0.15, 30.0),
//     ImageSolidCheck(0.806, 0.431, 0.043, 0.026, 0.350, 0.371, 0.278, 0.15, 20.0),
//     ImageSolidCheck(0.827, 0.507, 0.023, 0.028, 0.327, 0.386, 0.287, 0.15, 30.0),
//     ImageSolidCheck(0.747, 0.603, 0.087, 0.086, 0.402, 0.439, 0.159, 0.15, 20.0),
//     ImageSolidCheck(0.885, 0.653, 0.041, 0.093, 0.353, 0.404, 0.242, 0.15, 20.0),
//     ImageSolidCheck(0.934, 0.556, 0.039, 0.038, 0.413, 0.446, 0.141, 0.15, 30.0),
//     ImageSolidCheck(0.761, 0.913, 0.071, 0.051, 0.373, 0.424, 0.203, 0.15, 30.0),
//     ImageSolidCheck(0.865, 0.098, 0.022, 0.030, 0.326, 0.353, 0.321, 0.15, 50.0),
//     ImageSolidCheck(0.202, 0.631, 0.057, 0.075, 0.414, 0.468, 0.118, 0.15, 20.0),
//     ImageSolidCheck(0.099, 0.265, 0.072, 0.070, 0.288, 0.376, 0.336, 0.15, 20.0),
//     ImageSolidCheck(0.908, 0.313, 0.020, 0.017, 0.316, 0.387, 0.297, 0.15, 20.0),
// };


}


BlackOutDetector::BlackOutDetector(Logger& logger, VideoOverlay& overlay)
    : VisualInferenceCallback("BlackOutDetector")
    , m_black_screen(0.068, 0.088, 0.864, 0.581)
    , m_yellow_arrow_detector(logger, overlay, true)
    // , m_button_Y_detector(logger, overlay, ButtonType::ButtonY, ImageFloatBox{0.439, 0.819, 0.029, 0.059}, std::chrono::milliseconds(0), false)
{}

void BlackOutDetector::make_overlays(VideoOverlaySet& items) const{
    m_yellow_arrow_detector.make_overlays(items);
    items.add(COLOR_BLUE, m_black_screen);
}

//  Return true if the inference session should stop.
bool BlackOutDetector::process_frame(const ImageViewRGB32& frame, WallClock timestamp){

    auto save_image = [&](){
        static int count = 0;
        frame.save("BlackOut-" + std::to_string(count) + ".png");
        count++;
    };

    // check whether it is the black out screen
    const bool is_screen_black = is_black(extract_box_reference(frame, m_black_screen), 100, 10);
    if (is_screen_black && m_yellow_arrow_detector.process_frame(frame, timestamp)){
        // We have both a mostly black screen and a yellow arrow:
        save_image();
        return true;
    }

    return false;


    // // Check each patch of background color of the dropped item screen
    // for(const auto& check: dropped_item_bg_checks){
    //     if (check.check(frame) == false){
    //         // cout << "Check failed " << check.debug_string(frame) << endl;
    //         return false;
    //     }
    // }
    // save_image();
    // return true;
}



}
}
}
