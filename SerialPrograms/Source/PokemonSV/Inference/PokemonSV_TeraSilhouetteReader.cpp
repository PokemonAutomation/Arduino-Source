/*  Tera Silhouette Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <opencv2/imgproc.hpp>

#include "CommonFramework/ImageMatch/ImageCropper.h"
#include "CommonFramework/ImageTools/ImageFilter.h"
#include "PokemonSV/Resources/PokemonSV_PokemonSprites.h"

#include "PokemonSV_TeraSilhouetteReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


ImageMatch::SilhouetteDictionaryMatcher make_TERA_RAID_SILHOUETTE_MATCHER(){
    ImageMatch::SilhouetteDictionaryMatcher matcher;
    for (const auto& item : ALL_POKEMON_SILHOUETTES()){
        if (item.first == "pm1084_00_00_00_big" ||
            item.first == "pm1091_00_00_00_big" ||
            item.first == "error") {
            continue;
        }
        ImageRGB32 filtered_image = to_blackwhite_rgb32_range(item.second.icon, 0xff000000, 0xff5f5f5f, true);
        matcher.add(item.first, filtered_image);
    }
    return matcher;
}
const ImageMatch::SilhouetteDictionaryMatcher& TERA_RAID_SILHOUETTE_MATCHER(){
    static ImageMatch::SilhouetteDictionaryMatcher matcher = make_TERA_RAID_SILHOUETTE_MATCHER();
    return matcher;
}

TeraSilhouetteReader::TeraSilhouetteReader(Color color)
    : m_matcher(TERA_RAID_SILHOUETTE_MATCHER())
    , m_color(color)
    , m_box(0.536, 0.122, 0.252, 0.430)
{}

void TeraSilhouetteReader::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}

ImageMatch::ImageMatchResult TeraSilhouetteReader::read(const ImageViewRGB32& screen) const{
    static constexpr double MAX_ALPHA = 100;
    static constexpr double ALPHA_SPREAD = 20;

    // Get a loose crop of the silhouette icon
    ImageViewRGB32 cropped_image = extract_box_reference(screen, m_box);
    //cropped_image.save("cropped_image.png");

    ImageRGB32 preprocessed_image(cropped_image.width(), cropped_image.height());
    cv::medianBlur(cropped_image.to_opencv_Mat(), preprocessed_image.to_opencv_Mat(), 5);
    //preprocessed_image.save("preprocessed_image.png");

    // Get a tight crop
    const ImagePixelBox tight_box = ImageMatch::enclosing_rectangle_with_pixel_filter(
        preprocessed_image,
        // The filter is a lambda function that returns true on black silhouette pixels.
        [](Color pixel){
            return (uint32_t)pixel.red() + pixel.green() + pixel.blue() < 200;
        }
    );
    ImageRGB32 processed_image = extract_box_reference(preprocessed_image, tight_box).copy();
    //processed_image.save("processed_image.png");

    ImageRGB32 filtered_image = to_blackwhite_rgb32_range(processed_image, 0xff000000, 0xff5f5f5f, true);
    //filtered_image.save("filtered_image.png");

    ImageMatch::ImageMatchResult slugs = m_matcher.match(filtered_image, ALPHA_SPREAD);
    slugs.clear_beyond_alpha(MAX_ALPHA);

    return slugs;
}



}
}
}
