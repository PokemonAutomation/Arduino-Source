/*  Cropped Image Dictionary Matcher
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonTools_CroppedImageDictionaryMatcher_H
#define PokemonAutomation_CommonTools_CroppedImageDictionaryMatcher_H

#include <vector>
#include "ImageMatchResult.h"
#include "ExactImageMatcher.h"

namespace PokemonAutomation{
namespace ImageMatch{

// Similar to `ExactImageDictionaryMatcher` but will crop the image based on background pixel colors before matching.
class CroppedImageDictionaryMatcher{
public:
    CroppedImageDictionaryMatcher(
        const WeightedExactImageMatcher::InverseStddevWeight& weight = WeightedExactImageMatcher::InverseStddevWeight()
    );
    virtual ~CroppedImageDictionaryMatcher() = default;

    void add(const std::string& slug, const ImageViewRGB32& image);

    ImageMatchResult match(const ImageViewRGB32& image, double alpha_spread) const;


protected:
    //  Return potential crops for this image.
    virtual std::vector<ImageViewRGB32> get_crop_candidates(const ImageViewRGB32& image) const = 0;


private:
    WeightedExactImageMatcher::InverseStddevWeight m_weight;
    std::map<std::string, WeightedExactImageMatcher> m_database;
};


}
}
#endif
