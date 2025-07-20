/*  Silhouette Dictionary Matcher
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonTools_SilhouetteDictionaryMatcher_H
#define PokemonAutomation_CommonTools_SilhouetteDictionaryMatcher_H

//#include "Common/Compiler.h"
//#include "CommonFramework/ImageTools/FloatPixel.h"
#include "ImageMatchResult.h"
#include "ExactImageMatcher.h"

namespace PokemonAutomation{
    class ImageViewRGB32;
namespace ImageMatch{


// Match silhouette to several templates.
class SilhouetteDictionaryMatcher{
public:
    SilhouetteDictionaryMatcher(SilhouetteDictionaryMatcher&&) = default;
    SilhouetteDictionaryMatcher& operator=(SilhouetteDictionaryMatcher&&) = default;
    SilhouetteDictionaryMatcher(const SilhouetteDictionaryMatcher&) = delete;
    void operator=(const SilhouetteDictionaryMatcher&) = delete;

public:
    SilhouetteDictionaryMatcher() = default;
    virtual ~SilhouetteDictionaryMatcher() = default;

    // Add a silhouette template. The alpha==0 boundaries in the image will be trimmed when added.
    void add(const std::string& slug, const ImageViewRGB32& image);

    // Match the input image with the templates.
    // alpha_spread: used as tolerance for multiple match results.
    // Apart from the best march result, other weaker match results are also returned in `ImageMatchResult` if 
    // their scores are not larger than the best match score + `alpha_spread`.
    // Details of the match algorithm:
    // Resize image first to match the shape of the image template. Then scale the template brightness to match
    // the input image. Finally compute their RMSD (root mean square deviation).
    // Alpha channels from both the template and the input image are considered when computing RMSD.
    // If only one of the two has alpha==255 on one pixel, that the deviation on that pixel is the max pixel distance.
    // If both two images have alpha==0 on one pixel, that pixel is ignored.
    ImageMatchResult match(const ImageViewRGB32& image, double alpha_spread) const;


private:
    std::map<std::string, ExactImageMatcher> m_database;
    std::vector<const std::pair<const std::string, ExactImageMatcher>*> m_database_vector;
};


}
}
#endif
