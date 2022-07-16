/*  Pokemon Sprite Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/ImageTypes/RGB32ImageView.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageMatch/ImageCropper.h"
#include "PokemonSwSh/Resources/PokemonSwSh_PokemonSprites.h"
#include "PokemonSwSh_PokemonSpriteReader.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


PokemonSpriteMatcherExact::PokemonSpriteMatcherExact(const std::set<std::string>* subset)
    : ExactImageDictionaryMatcher({1, 256})
{
    for (const auto& item : ALL_POKEMON_SPRITES()){
        if (subset == nullptr || subset->find(item.first) != subset->end()){
//            cout << item.first << endl;
            add(item.first, item.second.sprite.to_qimage());
        }
    }
}
PokemonLeftSpriteMatcherExact::PokemonLeftSpriteMatcherExact(const std::set<std::string>* subset)
    : ExactImageDictionaryMatcher({1, 256})
{
    for (const auto& item : ALL_POKEMON_SPRITES()){
        if (subset == nullptr || subset->find(item.first) != subset->end()){
//            cout << item.first << endl;
            const QImage& sprite = item.second.sprite.to_qimage();
            int width = sprite.width();
            int height = sprite.height();
            add(item.first, sprite.copy(0, 0, width/2, height));
        }
    }
}






PokemonSpriteMatcherCropped::PokemonSpriteMatcherCropped(const std::set<std::string>* subset, double min_euclidean_distance)
    : CroppedImageDictionaryMatcher({1, 256})
    , m_min_euclidean_distance_squared(min_euclidean_distance * min_euclidean_distance)
{
    for (const auto& item : ALL_POKEMON_SPRITES()){
        if (subset == nullptr || subset->find(item.first) != subset->end()){
//            cout << item.first << endl;
            add(item.first, item.second.sprite.to_qimage());
        }
    }
}

QRgb PokemonSpriteMatcherCropped::crop_image(QImage& image) const{
    ImageStats border = image_border_stats(image);
//    cout << border.average << border.stddev << endl;
//    image.save("image1.png");
    QRect rect = ImageMatch::enclosing_rectangle_with_pixel_filter(
        image,
        [&](QRgb pixel){
//            if (qAlpha(pixel) == 0){
//                return false;
//            }
//            FloatPixel p(pixel);
//            cout << p << endl;
            double r = (double)qRed(pixel) - border.average.r;
            double g = (double)qGreen(pixel) - border.average.g;
            double b = (double)qBlue(pixel) - border.average.b;
            bool stop = r*r + g*g + b*b >= m_min_euclidean_distance_squared;
            if (stop){
//                FloatPixel p(pixel);
//                cout << p << " : " << r << " " << g << " " << b << endl;
            }
            return stop;
        }
    );
    image = image.copy(rect);
//    image.save("image2.png");
    return border.average.round();
}
void PokemonSpriteMatcherCropped::crop_sprite(QImage& image, QRgb background) const{
//    FloatPixel p(background);
//    cout << p << endl;
//    image.save("test0.png");
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
//            if (stop){
//                cout << r << " " << g << " " << b << endl;
//            }
            return stop;
        }
    );
    image = image.copy(rect);
}








}
}
}
