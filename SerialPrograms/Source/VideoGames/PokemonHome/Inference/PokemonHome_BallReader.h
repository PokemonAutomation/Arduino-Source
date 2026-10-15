/*  Ball Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonHome_BallReader_H
#define PokemonAutomation_PokemonHome_BallReader_H

#include <memory>
#include <string>
#include <vector>
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "CommonTools/ImageMatch/CroppedImageDictionaryMatcher.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{



struct SimilarBallGroup{
    std::vector<std::string> slugs;
    size_t comparison_width = 20;
    size_t comparison_height = 19;
    double crop_distance = 200;
    double coarse_spread = 0.08;
    double max_detail_alpha = 0.20;
    double detail_spread = 0.03;
    double min_pixel_distance = 30;
    size_t min_detail_pixels = 25;
};

class PokeballSpriteMatcher : public ImageMatch::CroppedImageDictionaryMatcher{
public:
    PokeballSpriteMatcher(double min_euclidean_distance = 100);
    PokeballSpriteMatcher(const std::vector<SimilarBallGroup>& groups, double min_euclidean_distance = 100);
    virtual ~PokeballSpriteMatcher();

    ImageMatch::ImageMatchResult match(const ImageViewRGB32& image, double alpha_spread) const;

private:
    static ImageRGB32 remove_white_border(const ImageViewRGB32& image);
    virtual std::vector<ImageViewRGB32> get_crop_candidates(const ImageViewRGB32& image) const override;

private:
    class SimilarBallGroupMatcher;
    double m_min_euclidean_distance_squared;
    std::vector<std::unique_ptr<SimilarBallGroupMatcher>> m_similar_groups;
};




class BallReader{
    static const double MAX_ALPHA;
    static const double ALPHA_SPREAD;

public:
    BallReader(VideoStream& stream);

public:
    std::string read_ball(const ImageViewRGB32& screen) const;

private:
    const PokeballSpriteMatcher& m_matcher;
    VideoStream& m_stream;
    OverlayBoxScope m_box_sprite;
};

void add_tests_BallReader(UnitTestDatabase& database);



}
}
}
#endif
