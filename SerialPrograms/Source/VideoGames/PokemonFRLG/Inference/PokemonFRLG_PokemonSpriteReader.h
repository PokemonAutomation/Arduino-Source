/*  Pokemon FRLG Sprite Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_PokemonSpriteReader_H
#define PokemonAutomation_PokemonFRLG_PokemonSpriteReader_H

#include <set>
#include "CommonTools/ImageMatch/ExactImageDictionaryMatcher.h"
#include "CommonTools/ImageMatch/CroppedImageDictionaryMatcher.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


class PokemonSpriteMatcherExact : public ImageMatch::ExactImageDictionaryMatcher{
public:
    PokemonSpriteMatcherExact(const std::set<std::string>* subset, bool flipped = true);
};


class PokemonSpriteMatcherCropped : public ImageMatch::CroppedImageDictionaryMatcher{
public:
    PokemonSpriteMatcherCropped(const std::set<std::string>* subset, double min_euclidean_distance = 100, bool flipped = true);

private:
    virtual std::vector<ImageViewRGB32> get_crop_candidates(const ImageViewRGB32& image) const override;

private:
    double m_min_euclidean_distance_squared;
};


class SummarySpriteReader{
public:
    SummarySpriteReader(const std::set<std::string>& subset, Color color = COLOR_RED);

    void make_overlays(VideoOverlaySet& items) const;

    ImageMatch::ImageMatchResult read(const ImageViewRGB32& frame);

private:
    Color m_color;
    ImageFloatBox m_box_sprite;
    PokemonSpriteMatcherCropped sprite_matcher;
    PokemonSpriteMatcherExact exact_sprite_matcher;
};


}
}
}
#endif
