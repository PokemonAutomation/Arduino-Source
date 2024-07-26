/*  Fast Travel Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/GlobalSettingsPanel.h"
#include "Common/Cpp/Containers/FixedLimitVector.tpp"
#include "CommonFramework/ImageMatch/WaterfillTemplateMatcher.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTools/WaterfillUtilities.h"
#include "CommonFramework/ImageTools/BinaryImage_FilterRgb32.h"
#include "CommonFramework/ImageTools/ImageManip.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Tools/DebugDumper.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "PokemonSV_DirectionDetector.h"

#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


class DirectionMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    
    /* 
    - selects for the white, wing-shaped component of the fast travel icon
    - Use Waterfilltemplate matcher to get your template so you get the correct area ratio
    along with the blue background. You need the blue backgroun so you don't false positive
    against the fly icon in Pokemon centers.

     */
    DirectionMatcher() : WaterfillTemplateMatcher(
        "PokemonSV/Map/DirectionIcon-Template.png", Color(0, 100, 30), Color(50, 230, 85), 50
    ){
        m_aspect_ratio_lower = 0.8;
        m_aspect_ratio_upper = 1.2;
        m_area_ratio_lower = 0.8;
        m_area_ratio_upper = 1.2;

    }

    static const ImageMatch::WaterfillTemplateMatcher& instance(){
        static DirectionMatcher matcher;
        return matcher;
    }
};


DirectionDetector::~DirectionDetector() = default;

DirectionDetector::DirectionDetector(Color color, const ImageFloatBox& box)
    : m_color(color)
    , m_minimap_box(box)
{}

void DirectionDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_minimap_box);
}

bool DirectionDetector::detect(const ImageViewRGB32& screen) const{
    return true;
}

std::pair<double, double> DirectionDetector::locate_north(const ImageViewRGB32& screen) const{
    const std::vector<std::pair<uint32_t, uint32_t>> filters = {
        {combine_rgb(0, 100, 0), combine_rgb(50, 230, 80)},

    };

    const double min_object_size = 50.0;
    const double rmsd_threshold = 60.0;

    const double screen_rel_size = (screen.height() / 1080.0);
    const size_t min_size = size_t(screen_rel_size * screen_rel_size * min_object_size);

    std::pair<double, double> north_location(-1.0, -1.0);

    #if 0
        std::vector<ImageFloatBox> candidate_locations = north_candidate_locations(screen);
        // std::vector<ImageRGB32> candidate_images = north_candidate_locations(screen); // vector<imagefloatbox>, filters
        for (ImageFloatBox& box : candidate_locations){
            ImagePixelBox pixel_search_area = floatbox_to_pixelbox(screen.width(), screen.height(), box);
            match_template_by_waterfill(
                extract_box_reference(screen, box),
                DirectionMatcher::instance(),
                filters,
                {min_size, SIZE_MAX},
                rmsd_threshold,
                [&](Kernels::Waterfill::WaterfillObject& object) -> bool {
                    north_location = std::make_pair(
                        (object.center_of_gravity_x() + pixel_search_area.min_x) / (double)screen.width(),
                        (object.center_of_gravity_y() + pixel_search_area.min_y) / (double)screen.height()
                    );
                    return true;
                }
            ); 
            if (north_location.first >= 0.0) {
                break;
            }          
        }     
    #endif   

    #if 0
        std::vector<ImageFloatBox> candidate_locations = north_candidate_locations(screen);
        // std::vector<ImageRGB32> candidate_images = north_candidate_locations(screen); // vector<imagefloatbox>, filters
        for (ImageFloatBox& box : candidate_locations){
            ImagePixelBox pixel_search_area = floatbox_to_pixelbox(screen.width(), screen.height(), box);
            std::vector<ImageRGB32> candidate_image = north_candidate_images(screen, box);
            for (ImageRGB32& candidate : candidate_image){
                match_template_by_waterfill(
                    candidate, 
                    DirectionMatcher::instance(),
                    filters,
                    {min_size, SIZE_MAX},
                    rmsd_threshold,
                    [&](Kernels::Waterfill::WaterfillObject& object) -> bool {
                        north_location = std::make_pair(
                            (object.center_of_gravity_x() + pixel_search_area.min_x) / (double)screen.width(),
                            (object.center_of_gravity_y() + pixel_search_area.min_y) / (double)screen.height()
                        );
                        return true;
                    }
                ); 
                if (north_location.first >= 0.0) {
                    break;
                }                
            }
            if (north_location.first >= 0.0) {
                break;
            }             
        }     
    #endif                

    #if 1
        ImagePixelBox pixel_search_area = floatbox_to_pixelbox(screen.width(), screen.height(), m_minimap_box);
        match_template_by_waterfill(
            extract_box_reference(screen, m_minimap_box), 
            DirectionMatcher::instance(),
            filters,
            {min_size, SIZE_MAX},
            rmsd_threshold,
            [&](Kernels::Waterfill::WaterfillObject& object) -> bool {
                north_location = std::make_pair(
                    (object.center_of_gravity_x() + pixel_search_area.min_x) / (double)screen.width(),
                    (object.center_of_gravity_y() + pixel_search_area.min_y) / (double)screen.height()
                );
                return true;
            }
        );
    #endif


    std::cout << "north location: " << std::to_string(north_location.first) << ", " << std::to_string(north_location.second) << std::endl;

    return north_location;
}



std::vector<ImageFloatBox> DirectionDetector::north_candidate_locations(const ImageViewRGB32& screen) const{
    const std::vector<std::pair<uint32_t, uint32_t>> filters = {
        // {combine_rgb(0, 180, 100), combine_rgb(50, 255, 150)},
        {combine_rgb(0, 180, 80), combine_rgb(85, 255, 175)},

    };

    const double min_object_size = 150.0;

    const double screen_rel_size = (screen.height() / 1080.0);
    const size_t min_size = size_t(screen_rel_size * screen_rel_size * min_object_size);

    std::vector<ImageFloatBox> candidate_locations = match_filters_by_waterfill(
                                                    screen,
                                                    m_minimap_box,
                                                    filters,
                                                    {min_size, SIZE_MAX}
                                                );    
    return candidate_locations;                                                
}

// use the color filters to select for the bright-green portion in the overworld compass
// then from these candidates, run a binary filter to keep the bright-green portion, and 
// replace everything else with dark green.
// since the N was originally dark green, this binary filter essentially removes all of the
// extraneous pixels outside of the bright-green portion of the overworld compass.
std::vector<ImageRGB32> DirectionDetector::north_candidate_images(const ImageViewRGB32& screen, const ImageFloatBox& box) const{
    const std::vector<std::pair<uint32_t, uint32_t>> filters = {
        // {combine_rgb(0, 180, 100), combine_rgb(50, 255, 150)},
        {combine_rgb(0, 180, 80), combine_rgb(85, 255, 175)},

    };

    std::vector<ImageRGB32> candidate_images;
    const ImageViewRGB32 &candidate_image = pad_image(extract_box_reference(screen, box), 10, 0xffffffff);
    auto matrices = compress_rgb32_to_binary_range(candidate_image, filters);
    for(PokemonAutomation::PackedBinaryMatrix &matrix : matrices){
        ImageRGB32 binaryImage = pad_image(extract_box_reference(screen, box), 10, 0xffffffff);
        filter_by_mask(matrix, binaryImage, Color(10, 190, 70), true);
        if (PreloadSettings::debug().IMAGE_TEMPLATE_MATCHING){
            dump_debug_image(
                global_logger_command_line(), 
                "CommonFramework/WaterfillTemplateMatcher", 
                "binary_image_candidate", 
                binaryImage);                
        }
        candidate_images.emplace_back(std::move(binaryImage));
    }
                                                  
    return candidate_images;  

}

int16_t DirectionDetector::current_direction(const ImageViewRGB32& screen) const{
    return 0;
}



DirectionWatcher::~DirectionWatcher() = default;

DirectionWatcher::DirectionWatcher(Color color, VideoOverlay& overlay, const ImageFloatBox& box)
    : VisualInferenceCallback("DirectionWatcher")
    , m_overlay(overlay)
    , m_detector(color, box)
{}

void DirectionWatcher::make_overlays(VideoOverlaySet& items) const{
    m_detector.make_overlays(items);
}

bool DirectionWatcher::process_frame(const ImageViewRGB32& screen, WallClock timestamp){
    return true;
}








}
}
}
