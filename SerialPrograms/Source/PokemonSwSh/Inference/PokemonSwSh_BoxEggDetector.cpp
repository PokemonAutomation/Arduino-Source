/*  Box Egg Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/ImageMatch/ImageCropper.h"
#include "CommonTools/ImageMatch/ExactImageMatcher.h"
#include "PokemonSwSh/Resources/PokemonSwSh_PokemonSprites.h"
#include "PokemonSwSh_BoxEggDetector.h"

// #include <iostream>
// using std::cout;
// using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{



const ImageMatch::ExactImageMatcher& EGG_MATCHER(){
    static ImageMatch::ExactImageMatcher matcher(RESOURCE_PATH() + "PokemonSwSh/EggTemplate.png");
    return matcher;
}

BoxEggDetector::BoxEggDetector(SlotLocation side, uint8_t row, double min_euclidean_distance, Color color)
: m_min_euclidean_distance_squared(min_euclidean_distance * min_euclidean_distance)
, m_color(color){
    if (side == SlotLocation::PARTY){
        if (row > 5){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "INVALID party row for BoxEggDetector");
        }
        m_box = ImageFloatBox(0.045414, 0.1255 * row + 0.2401, 0.023882, 0.043856);
    }else if (side == SlotLocation::BOX){
        if (row > 4){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "INVALID box row for BoxEggDetector");
        }
        m_box = ImageFloatBox(0.270241, 0.1255 * row + 0.2401, 0.023882, 0.049);
    }else{
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "INVALID SlotLocation for BoxEggDetector");
    }
}

void BoxEggDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}

bool BoxEggDetector::detect(const ImageViewRGB32& screen){

    std::vector<ImageViewRGB32> crops = get_crop_candidates(extract_box_reference(screen, m_box));

    // int i = 0;
    double rmsd_threshold = 80;
    for (const ImageViewRGB32& crop : crops){
        // crop.save("image" + std::to_string(i) + ".png");
        // i++;
        double rmsd = EGG_MATCHER().rmsd(crop);
        // cout << "RMSD: " << std::to_string(rmsd) << endl;
        if (rmsd < rmsd_threshold){
            return true;
        }
    }

    return false;
}

auto BoxEggDetector::get_crop_candidates(const ImageViewRGB32& image) const -> std::vector<ImageViewRGB32>{
    ImageStats border = image_border_stats(image);
//    cout << border.average << border.stddev << endl;
//    image.save("image1.png");
    ImagePixelBox box = ImageMatch::enclosing_rectangle_with_pixel_filter(
        image,
        [&](Color pixel){
//            if (qAlpha(pixel) == 0){
//                return false;
//            }
//            FloatPixel p(pixel);
//            cout << p << endl;
            double r = (double)pixel.red() - border.average.r;
            double g = (double)pixel.green() - border.average.g;
            double b = (double)pixel.blue() - border.average.b;
            bool stop = r*r + g*g + b*b >= m_min_euclidean_distance_squared;
            if (stop){
//                FloatPixel p(pixel);
//                cout << p << " : " << r << " " << g << " " << b << endl;
            }
            return stop;
        }
    );
    std::vector<ImageViewRGB32> ret;
    ret.emplace_back(extract_box_reference(image, box));
    return ret;
}









}
}
}
