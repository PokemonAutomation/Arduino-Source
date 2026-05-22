/*  Pokemon FRLG Sprite Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonTools/ImageMatch/ImageCropper.h"
#include "PokemonFRLG/Resources/PokemonFRLG_PokemonSprites.h"
#include "PokemonFRLG/PokemonFRLG_Settings.h"
#include "PokemonFRLG_PokemonSpriteReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


PokemonSpriteMatcherExact::PokemonSpriteMatcherExact(const std::set<std::string>* subset, bool flipped)
    : ExactImageDictionaryMatcher({1, 256})
{
    for (const auto& item : (flipped ? FLIPPED_POKEMON_SPRITES() : ALL_POKEMON_SPRITES())){
        if (subset == nullptr || subset->find(item.first) != subset->end()){
            add(item.first, item.second.sprite.copy());
        }
    }
}


PokemonSpriteMatcherCropped::PokemonSpriteMatcherCropped(const std::set<std::string>* subset, double min_euclidean_distance, bool flipped)
    : CroppedImageDictionaryMatcher({1, 256})
    , m_min_euclidean_distance_squared(min_euclidean_distance * min_euclidean_distance)
{
    for (const auto& item : (flipped ? FLIPPED_POKEMON_SPRITES() : ALL_POKEMON_SPRITES())){
        if (subset == nullptr || subset->find(item.first) != subset->end()){
            add(item.first, item.second.sprite);
        }
    }
}

auto PokemonSpriteMatcherCropped::get_crop_candidates(const ImageViewRGB32& image) const -> std::vector<ImageViewRGB32>{
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


SummarySpriteReader::SummarySpriteReader(const std::set<std::string>& subset, Color color)
    : m_color(color), 
    m_box_sprite(0.113462, 0.209375, 0.274519, 0.381490),
    sprite_matcher(&subset, 100, true),
    exact_sprite_matcher(&subset, true)
{}

void SummarySpriteReader::make_overlays(VideoOverlaySet &items) const {
    const BoxOption &GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(m_color, GAME_BOX.inner_to_outer(m_box_sprite));
}

ImageMatch::ImageMatchResult SummarySpriteReader::read(const ImageViewRGB32& frame){
    static const double CROPPED_ALPHA_SPREAD = 0.03;
    static const double EXACT_ALPHA_SPREAD = 0.02;

    static const double RETRY_ALPHA = 0.25;

    ImageViewRGB32 game_screen =
        extract_box_reference(frame, GameSettings::instance().GAME_BOX);

    ImageViewRGB32 sprite_image = extract_box_reference(game_screen, m_box_sprite);

    ImageMatch::ImageMatchResult result = sprite_matcher.match(sprite_image, CROPPED_ALPHA_SPREAD);
    if (result.results.size() != 1 || result.results.begin()->first > RETRY_ALPHA){
        result = exact_sprite_matcher.match(
            sprite_image, m_box_sprite,
            5, EXACT_ALPHA_SPREAD
        );
    }

    return result;
}



}
}
}
