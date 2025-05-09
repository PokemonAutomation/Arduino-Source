/*  Tera Silhouette Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <opencv2/imgproc.hpp>
#include "CommonFramework/Logging/Logger.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonTools/ImageMatch/ImageCropper.h"
#include "PokemonSV/Resources/PokemonSV_PokemonSprites.h"
#include "PokemonSV_TeraSilhouetteReader.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


ImageMatch::SilhouetteDictionaryMatcher make_TERA_RAID_SILHOUETTE_MATCHER(){
    ImageMatch::SilhouetteDictionaryMatcher matcher;
    for (const auto& item : ALL_POKEMON_SILHOUETTES()){
        if (item.first == "pm1084_00_00_00_big" ||
            item.first == "pm1091_00_00_00_big" ||
            item.first == "error"){
            continue;
        }
        ImageRGB32 filtered_image = to_blackwhite_rgb32_range(
            item.second.icon,
            true,
            0xff000000, 0xff5f5f5f
        );
        matcher.add(item.first, filtered_image);
    }
    return matcher;
}
const ImageMatch::SilhouetteDictionaryMatcher& TERA_RAID_SILHOUETTE_MATCHER(){
    static ImageMatch::SilhouetteDictionaryMatcher matcher = make_TERA_RAID_SILHOUETTE_MATCHER();
    return matcher;
}

TeraSilhouetteReader::TeraSilhouetteReader(Color color)
    : m_color(color)
    , m_box(0.536, 0.122, 0.252, 0.430)
{}

void TeraSilhouetteReader::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}

ImageMatch::ImageMatchResult TeraSilhouetteReader::read(const ImageViewRGB32& screen) const{
    static constexpr double MAX_ALPHA = 120;
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

    ImageMatch::ImageMatchResult slugs;
    for (uint32_t threshold : BRIGHTNESS_THRESHOLDS){
//        cout << "check0" << endl;
        //  Get a loose crop of the silhouette icon
        ImageViewRGB32 cropped_image = extract_box_reference(screen, m_box);
//        cropped_image.save("tera_cropped_image-" + std::to_string(c++) + ".png");

//        cout << "check1" << endl;
        ImageRGB32 preprocessed_image(cropped_image.width(), cropped_image.height());
        cv::medianBlur(cropped_image.to_opencv_Mat(), preprocessed_image.to_opencv_Mat(), 5);
//        preprocessed_image.save("tera_blurred_image.png");

        //  Get a tight crop
//        cout << "check2" << endl;
        const ImagePixelBox tight_box = ImageMatch::enclosing_rectangle_with_pixel_filter(
            preprocessed_image,
            // The filter is a lambda function that returns true on black silhouette pixels.
            [=](Color pixel){
                return (uint32_t)pixel.red() + pixel.green() + pixel.blue() <= threshold;
            }
        );

        if (tight_box.area() == 0){
//            global_logger_tagged().log("TeraSilhouetteReader::read(): Cropped image is empty.", COLOR_RED);
            continue;
        }

//        cout << "check3" << endl;
        ImageRGB32 processed_image = extract_box_reference(preprocessed_image, tight_box).copy();
//        processed_image.save("tera_processed_image-" + std::to_string(c++) + ".png");

//        cout << "check4" << endl;
//        ImageRGB32 filtered_image = to_blackwhite_rgb32_range(processed_image, true, 0xff000000, 0xff5f5f5f);
        ImageRGB32 filtered_image = to_blackwhite_rgb32_brightness(
            processed_image, true,
            0x00010101, 0, threshold
        );
//        filtered_image.save("tera_filtered_image-" + std::to_string(c++) + ".png");

//        cout << "check5" << endl;
        slugs = TERA_RAID_SILHOUETTE_MATCHER().match(filtered_image, ALPHA_SPREAD);

        slugs.clear_beyond_alpha(MAX_ALPHA);

//        slugs.log(global_logger_tagged(), MAX_ALPHA);

        if (slugs.results.size() == 1){
            return slugs;
        }
    }

    return slugs;
}



}
}
}
