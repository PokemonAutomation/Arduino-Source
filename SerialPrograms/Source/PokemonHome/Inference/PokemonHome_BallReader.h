/*  Ball Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonHome_BallReader_H
#define PokemonAutomation_PokemonHome_BallReader_H

#include <string>
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "CommonTools/ImageMatch/CroppedImageDictionaryMatcher.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{



class PokeballSpriteMatcher : public ImageMatch::CroppedImageDictionaryMatcher{
public:
    PokeballSpriteMatcher(double min_euclidean_distance = 100);

private:
    static ImageRGB32 remove_white_border(const ImageViewRGB32& image);
    virtual std::vector<ImageViewRGB32> get_crop_candidates(const ImageViewRGB32& image) const override;

private:
    double m_min_euclidean_distance_squared;
};




class BallReader{
    static const double MAX_ALPHA;
    static const double ALPHA_SPREAD;

public:
    BallReader(VideoStream& stream);

public:
    std::string read_ball(const ImageViewRGB32& screen) const;

private:
    const ImageMatch::CroppedImageDictionaryMatcher& m_matcher;
    VideoStream& m_stream;
    OverlayBoxScope m_box_sprite;
};




}
}
}
#endif
