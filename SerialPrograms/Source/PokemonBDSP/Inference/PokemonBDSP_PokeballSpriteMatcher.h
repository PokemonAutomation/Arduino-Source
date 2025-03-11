/*  Pokeball Sprite Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_PokeballSpriteReader_H
#define PokemonAutomation_PokemonBDSP_PokeballSpriteReader_H

#include "CommonTools/ImageMatch/CroppedImageDictionaryMatcher.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class PokeballSpriteMatcher : public ImageMatch::CroppedImageDictionaryMatcher{
public:
    PokeballSpriteMatcher(double min_euclidean_distance = 100);

private:
    virtual std::vector<ImageViewRGB32> get_crop_candidates(const ImageViewRGB32& image) const override;

private:
    double m_min_euclidean_distance_squared;
};



}
}
}
#endif
