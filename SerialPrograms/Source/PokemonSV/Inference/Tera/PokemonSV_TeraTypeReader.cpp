/*  Tera Type Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonTools/Images/ImageFilter.h"
#include "CommonTools/ImageMatch/ImageCropper.h"
#include "PokemonSV/Resources/PokemonSV_PokemonSprites.h"
#include "PokemonSV_TeraTypeReader.h"

#include "CommonFramework/Logging/Logger.h"
//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



ImageMatch::SilhouetteDictionaryMatcher make_TERA_RAID_TYPE_MATCHER(){
    ImageMatch::SilhouetteDictionaryMatcher matcher;
    for (size_t i = 0; i < NUM_TERA_TYPE; i++){
        matcher.add(TERA_TYPE_NAMES[i], ALL_TERA_TYPE_ICONS()[i]);
    }
    return matcher;
}
const ImageMatch::SilhouetteDictionaryMatcher& TERA_RAID_TYPE_MATCHER(){
    static ImageMatch::SilhouetteDictionaryMatcher matcher = make_TERA_RAID_TYPE_MATCHER();
    return matcher;
}

TeraTypeReader::TeraTypeReader(Color color)
    : m_matcher(TERA_RAID_TYPE_MATCHER())
    , m_color(color)
    , m_box(0.787, 0.139, 0.073, 0.136)
{}
void TeraTypeReader::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}

ImageMatch::ImageMatchResult TeraTypeReader::read(const ImageViewRGB32& screen) const{
    static constexpr double MAX_ALPHA = 100;
    static constexpr double ALPHA_SPREAD = 20;

    const std::vector<uint32_t> BRIGHTNESS_THRESHOLDS{
        200,
        150,
        100,
        125,
        175,
        225,
    };

//    static int c = 0;
    for (uint32_t threshold : BRIGHTNESS_THRESHOLDS){

        // Get a loose crop of the tera type icon
        ImageViewRGB32 cropped_image = extract_box_reference(screen, m_box);
        //cropped_image.save("cropped_image.png");

        // Get a tight crop
        const ImagePixelBox tight_box = ImageMatch::enclosing_rectangle_with_pixel_filter(
            cropped_image,
            // The filter is a lambda function that returns true on black tera type pixels.
            [=](Color pixel){
                return (uint32_t)pixel.red() + pixel.green() + pixel.blue() <= threshold;
            }
        );

        if (tight_box.area() == 0){
            continue;
        }

        ImageRGB32 processed_image = extract_box_reference(cropped_image, tight_box).copy();
        processed_image.save("processed_image-" + std::to_string(threshold) + ".png");

        ImageRGB32 filtered_image = to_blackwhite_rgb32_brightness(
            processed_image, true,
            0x00010101, 0, threshold
        );
        filtered_image.save("filtered_image-" + std::to_string(threshold) + ".png");

        ImageMatch::ImageMatchResult types = m_matcher.match(filtered_image, ALPHA_SPREAD);
//        types.log(global_logger_tagged(), MAX_ALPHA);
        types.clear_beyond_alpha(MAX_ALPHA);

        if (types.results.size() == 1){
            return types;
        }
    }

    return ImageMatch::ImageMatchResult();
}



}
}
}
