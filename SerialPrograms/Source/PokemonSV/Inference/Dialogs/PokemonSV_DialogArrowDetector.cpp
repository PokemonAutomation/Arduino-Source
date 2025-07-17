/*  Dialog Arrow Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTypes/BinaryImage.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/BinaryImage_FilterRgb32.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "CommonTools/ImageMatch/ExactImageMatcher.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "PokemonSV_DialogArrowDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Kernels;
using namespace Kernels::Waterfill;

class DialogArrowMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    
    DialogArrowMatcher() : WaterfillTemplateMatcher(
        // "PokemonSV/DialogArrow-White-BlackBackground.png", Color(140, 140, 140), Color(255, 255, 255), 50
        "PokemonSV/DialogArrow-White-NoBackground.png", Color(140, 140, 140), Color(255, 255, 255), 50
    ){
        m_aspect_ratio_lower = 0.8;
        m_aspect_ratio_upper = 1.2;
        m_area_ratio_lower = 0.8;
        m_area_ratio_upper = 1.3;

    }

    static const ImageMatch::WaterfillTemplateMatcher& instance(){
        static DialogArrowMatcher matcher;
        return matcher;
    }
};

const ImageMatch::ExactImageMatcher& DIALOG_ARROW_BLACK(){
    static ImageMatch::ExactImageMatcher matcher(RESOURCE_PATH() + "PokemonSV/DialogArrow-Black.png");
    return matcher;
}
const ImageMatch::ExactImageMatcher& DIALOG_ARROW_WHITE(){
    static ImageMatch::ExactImageMatcher matcher(RESOURCE_PATH() + "PokemonSV/DialogArrow-White.png");
    return matcher;
}

bool is_dialog_arrow(const ImageViewRGB32& image, const WaterfillObject& object, bool black_arrow){
    double aspect_ratio = object.aspect_ratio();
//    cout << "aspect_ratio = " << aspect_ratio << endl;
    if (aspect_ratio < 1.1 || aspect_ratio > 1.4){
        return false;
    }
//    double area = (double)object.area_ratio();
//    if (area < 0.4 || area > 0.5){
//        return false;
//    }

    ImageViewRGB32 cropped = extract_box_reference(image, object);
//    cropped.save("test.png");

    double rmsd = black_arrow
        ? DIALOG_ARROW_BLACK().rmsd(cropped)
        : DIALOG_ARROW_WHITE().rmsd(cropped);
    return rmsd <= 120;
}



DialogArrowDetector::DialogArrowDetector(Color color, const ImageFloatBox& box)
    : m_color(color)
    , m_box(box)
{}
void DialogArrowDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}
bool DialogArrowDetector::detect(const ImageViewRGB32& screen){
    std::vector<ImageFloatBox> hits = detect_all(screen);
    return !hits.empty();
}

std::vector<ImageFloatBox> DialogArrowDetector::detect_all(const ImageViewRGB32& screen) const{
    using namespace Kernels::Waterfill;

    ImageViewRGB32 region = extract_box_reference(screen, m_box);

    std::vector<ImageFloatBox> hits;

    {
        PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(region, 0xff000000, 0xff7f7fbf);
        std::unique_ptr<WaterfillSession> session = make_WaterfillSession(matrix);
        auto iter = session->make_iterator(20);
        WaterfillObject object;
        while (iter->find_next(object, false)){
            if (is_dialog_arrow(region, object, true)){
                hits.emplace_back(translate_to_parent(screen, m_box, object));
            }
        }
    }
    {
        PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(region, 0xff808080, 0xffffffff);
        std::unique_ptr<WaterfillSession> session = make_WaterfillSession(matrix);
        auto iter = session->make_iterator(20);
        WaterfillObject object;
        while (iter->find_next(object, false)){
            if (is_dialog_arrow(region, object, false)){
                hits.emplace_back(translate_to_parent(screen, m_box, object));
            }
        }
    }

    return hits;
}


std::pair<double, double> DialogArrowDetector::locate_dialog_arrow(const ImageViewRGB32& screen) const{
    const std::vector<std::pair<uint32_t, uint32_t>> filters = {
        {combine_rgb(200, 200, 200), combine_rgb(255, 255, 255)},

    };

    ImageViewRGB32 cropped = extract_box_reference(screen, m_box);
    // ImageRGB32 binaryImage = cropped.copy();
    // PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(cropped, 0xffC8C8C8, 0xffffffff);
    // filter_by_mask(matrix, binaryImage, Color(COLOR_BLACK), true);

    const double min_object_size = 50.0;
    const double rmsd_threshold = 80.0;

    const double screen_rel_size = (screen.height() / 1080.0);
    const size_t min_size = size_t(screen_rel_size * screen_rel_size * min_object_size);

    std::pair<double, double> arrow_location(-1.0, -1.0);

    ImagePixelBox pixel_search_area = floatbox_to_pixelbox(screen.width(), screen.height(), m_box);
    match_template_by_waterfill(
        cropped, 
        DialogArrowMatcher::instance(),
        filters,
        {min_size, SIZE_MAX},
        rmsd_threshold,
        [&](Kernels::Waterfill::WaterfillObject& object) -> bool {
            
            arrow_location = std::make_pair(
                (object.center_of_gravity_x() + pixel_search_area.min_x) / (double)screen.width(),
                (object.center_of_gravity_y() + pixel_search_area.min_y) / (double)screen.height()
            );

            return true;
        }
    );

    // std::cout << "north location: " << std::to_string(north_location.first) << ", " << std::to_string(north_location.second) << std::endl;

    return arrow_location;
}



DialogArrowWatcher::~DialogArrowWatcher() = default;
DialogArrowWatcher::DialogArrowWatcher(Color color, VideoOverlay& overlay, const ImageFloatBox& box, const double top_line, const double bottom_line)
    : VisualInferenceCallback("GradientArrowFinder")
    , m_overlay(overlay)
    , m_detector(color, box)
    , m_top_line(top_line)
    , m_bottom_line(bottom_line)
    , m_num_oscillation_above_top_line(0)
    , m_num_oscillation_below_bottom_line(0)
{}

void DialogArrowWatcher::make_overlays(VideoOverlaySet& items) const{
    m_detector.make_overlays(items);
}

// - return true if detects the arrow for 5 up and down oscillations
// - every time the arrow is above top_line, increment m_num_oscillation_above_top_line. 
// - likewise for m_num_oscillation_below_bottom_line
// - we alternate between looking for the arrow being above top_line vs below bottom_line
// - reset counts whenever the dialog arrow has not been detected for 10 frames consecutively
bool DialogArrowWatcher::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    std::pair<double, double> arrow_location = m_detector.locate_dialog_arrow(frame);
    double y_location = arrow_location.second;
    // cout << std::to_string(y_location) << endl;

    if (y_location < 0){ // dialog arrow not detected
        m_num_no_detection++;
        if (m_num_no_detection > 10){
            // reset oscillation counts
            m_num_oscillation_above_top_line = 0;
            m_num_oscillation_below_bottom_line = 0;
        }
        return false;        
    }else{
        m_num_no_detection = 0;
    }
    
    if (m_num_oscillation_above_top_line >= 5 && m_num_oscillation_below_bottom_line >= 5){
        // we have had 5 oscillations above and below the top and bottom line respectively
        return true;
    }

    if (m_num_oscillation_above_top_line < m_num_oscillation_below_bottom_line){
        // watch for the arrow above the top_line
        if (y_location < m_top_line){ // remember that 0,0 is the top left corner. so being above a line, means less-than
            // cout << "above top line." << endl;
            m_num_oscillation_above_top_line++;
        }
    }else{
        // watch for the arrow below the bottom_line
        if (y_location > m_bottom_line){
            // cout << "below bottom line." << endl;
            m_num_oscillation_below_bottom_line++;
        }        
    }

    return false;
}








}
}
}
