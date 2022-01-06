/*  Pokeball Sprite Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

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
    for (const auto& item : PokemonSwSh::all_pokeball_sprites()){
        add(item.first, item.second.sprite());
    }
}

QRgb PokeballSpriteMatcher::crop_image(QImage& image) const{
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
    image = image.copy(rect);
    return border.average.round();
}
void PokeballSpriteMatcher::crop_sprite(QImage& image, QRgb background) const{
    QRect rect = ImageMatch::enclosing_rectangle_with_pixel_filter(
        image,
        [&](QRgb pixel){
            if (qAlpha(pixel) == 0){
                return false;
            }
            double r = (double)qRed(pixel) - (double)qRed(background);
            double g = (double)qGreen(pixel) - (double)qGreen(background);
            double b = (double)qBlue(pixel) - (double)qBlue(background);
            bool stop = r*r + g*g + b*b >= m_min_euclidean_distance_squared;
            return stop;
        }
    );
    image = image.copy(rect);
}



}
}
}
