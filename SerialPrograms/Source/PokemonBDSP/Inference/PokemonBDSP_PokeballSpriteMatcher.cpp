/*  Pokeball Sprite Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonTools/ImageMatch/ImageCropper.h"
#include "PokemonSwSh/Resources/PokemonSwSh_PokeballSprites.h"
#include "PokemonBDSP_PokeballSpriteMatcher.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


PokeballSpriteMatcher::PokeballSpriteMatcher(double min_euclidean_distance)
    : CroppedImageDictionaryMatcher({1, 128})
    , m_min_euclidean_distance_squared(min_euclidean_distance * min_euclidean_distance)
{
    for (const auto& item : PokemonSwSh::ALL_POKEBALL_SPRITES()){
        add(item.first, item.second.sprite);
    }
}
auto PokeballSpriteMatcher::get_crop_candidates(const ImageViewRGB32& image) const -> std::vector<ImageViewRGB32>{
    ImageStats border = image_border_stats(image);
    ImagePixelBox box = ImageMatch::enclosing_rectangle_with_pixel_filter(
        image,
        [&](Color pixel){
            double r = (double)pixel.red() - border.average.r;
            double g = (double)pixel.green() - border.average.g;
            double b = (double)pixel.blue() - border.average.b;
            bool stop = r*r + g*g + b*b >= m_min_euclidean_distance_squared;
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
