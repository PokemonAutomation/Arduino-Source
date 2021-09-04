/*  Pokemon Sprite Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageMatch/ImageCropper.h"
#include "CommonFramework/ImageMatch/CroppedImageMatcher.h"
#include "PokemonSwSh/Resources/PokemonSwSh_PokemonSprites.h"
#include "PokemonSwSh_PokemonSpriteReader.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{



class PokemonSpriteMatcher : public ImageMatch::CroppedImageMatcher{
public:
    PokemonSpriteMatcher(double min_euclidean_distance)
        : CroppedImageMatcher(true)
        , m_min_euclidean_distance_squared(min_euclidean_distance * min_euclidean_distance)
    {}

    virtual QRgb crop_image(QImage& image) const override{
        ImageStats border = image_border_stats(image);
        QRect rect = ImageMatch::enclosing_rectangle_with_pixel_filter(
            image,
            [&](QRgb pixel){
//                if (qAlpha(pixel) == 0){
//                    return false;
//                }
                double r = (double)qRed(pixel) - border.average.r;
                double g = (double)qGreen(pixel) - border.average.g;
                double b = (double)qBlue(pixel) - border.average.b;
                bool stop = r*r + g*g + b*b >= m_min_euclidean_distance_squared;
//                if (stop){
//                    cout << r << " " << g << " " << b << endl;
//                }
                return stop;
            }
        );
        image = image.copy(rect);
        return border.average.round();
    }
    virtual void crop_sprite(QImage& image, QRgb background) const override{
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
//                if (stop){
//                    cout << r << " " << g << " " << b << endl;
//                }
                return stop;
            }
        );
        image = image.copy(rect);
    }

private:
    double m_min_euclidean_distance_squared;
};



PokemonSpriteMatcher make_POKEMON_SPRITE_MATCHER(){
    PokemonSpriteMatcher matcher(100);
    for (const auto& item : all_pokemon_sprites()){
        matcher.add(item.first, item.second.sprite());
    }
    return matcher;
}
const PokemonSpriteMatcher& POKEMON_SPRITE_MATCHER(){
    static PokemonSpriteMatcher matcher = make_POKEMON_SPRITE_MATCHER();
    return matcher;
}



ImageMatch::MatchResult read_pokemon_sprite_on_solid(
    Logger& logger,
    const QImage& image,
    double max_RMSD
){
    ImageMatch::MatchResult results = POKEMON_SPRITE_MATCHER().match(image);
    results.log(logger, max_RMSD);
    return results;
}



}
}
}
