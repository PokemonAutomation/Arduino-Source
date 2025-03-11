/*  Pokemon Sprite Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonTools/ImageMatch/ImageCropper.h"
#include "PokemonSwSh/Resources/PokemonSwSh_PokemonSprites.h"
#include "PokemonSwSh_PokemonSpriteReader.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


PokemonSpriteMatcherExact::PokemonSpriteMatcherExact(const std::set<std::string>* subset)
    : ExactImageDictionaryMatcher({1, 256})
{
    for (const auto& item : ALL_POKEMON_SPRITES()){
        if (subset == nullptr || subset->find(item.first) != subset->end()){
//            cout << item.first << endl;
            add(item.first, item.second.sprite.copy());
        }
    }
}
PokemonLeftSpriteMatcherExact::PokemonLeftSpriteMatcherExact(const std::set<std::string>* subset)
    : ExactImageDictionaryMatcher({1, 256})
{
    for (const auto& item : ALL_POKEMON_SPRITES()){
        if (subset == nullptr || subset->find(item.first) != subset->end()){
//            cout << item.first << endl;
            const ImageViewRGB32& sprite = item.second.sprite;
            size_t width = sprite.width();
            size_t height = sprite.height();
            add(item.first, sprite.sub_image(0, 0, width/2, height).copy());
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
            add(item.first, item.second.sprite);
        }
    }
}

auto PokemonSpriteMatcherCropped::get_crop_candidates(const ImageViewRGB32& image) const -> std::vector<ImageViewRGB32>{
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
