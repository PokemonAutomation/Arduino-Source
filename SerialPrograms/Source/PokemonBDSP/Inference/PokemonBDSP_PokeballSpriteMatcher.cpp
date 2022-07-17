/*  Pokeball Sprite Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageMatch/ImageCropper.h"
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


ImageRGB32 PokeballSpriteMatcher::process_image(const ImageViewRGB32& image, QRgb& background) const{
    ImageStats border = image_border_stats(image);
    QRect rect = ImageMatch::enclosing_rectangle_with_pixel_filter(
        image,
        [&](QRgb pixel){
            double r = (double)qRed(pixel) - border.average.r;
            double g = (double)qGreen(pixel) - border.average.g;
            double b = (double)qBlue(pixel) - border.average.b;
            bool stop = r*r + g*g + b*b >= m_min_euclidean_distance_squared;
            return stop;
        }
    );
    background = border.average.round();
    return image.sub_image(rect.x(), rect.y(), rect.width(), rect.height()).copy();
}



}
}
}
