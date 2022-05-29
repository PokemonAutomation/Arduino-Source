/*  Exact Image Dictionary Matcher
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_ExactImageDictionaryMatcher_H
#define PokemonAutomation_CommonFramework_ExactImageDictionaryMatcher_H

#include <string>
#include <map>
#include "CommonFramework/Logging/LoggerQt.h"
#include "ImageMatchResult.h"
#include "ExactImageMatcher.h"

namespace PokemonAutomation{
    struct ImageFloatBox;
namespace ImageMatch{


// Build a dictionary of image templates and use them to match against images.
// All the image templates must have the same image shape.
class ExactImageDictionaryMatcher{
public:
    ExactImageDictionaryMatcher(const WeightedExactImageMatcher::InverseStddevWeight& weight);

    // Add an image template.
    // Do not allow one slug to have more than one template.
    void add(const std::string& slug, QImage image_template);

    QSize dimensions() const{ return m_dimensions; }

    // Scale image to match the size of the templates.
    void scale_to_dimensions(QImage& image) const;

    // Match the `box` area on `image` against the stored template dictionary.
    // `tolerance`: how much translation noise to tolerate. tolerance of 1 means to allow 1 pixel
    //   off on the template when matching.
    // `alpha_spread`: only retain match results that no larger than the best match score +
    //    `max_alpha_spread`.
    //
    // Note: the dictionary must contain at least one template.
    // The input image area will be scaled to the template shape before matching.
    // The brightness of the input image and the stddev of the template is compensated during
    // matching. 
    ImageMatchResult match(
        const ConstImageRef& image, const ImageFloatBox& box,
        size_t tolerance,
        double alpha_spread
    ) const;


private:
    static double compare(
        const WeightedExactImageMatcher& sprite,
        const std::vector<QImage>& images
    );


private:
    WeightedExactImageMatcher::InverseStddevWeight m_weight;
    // The size of the image templates.
    // Each template must have the same size.
    QSize m_dimensions;
    std::map<std::string, WeightedExactImageMatcher> m_database;
};



}
}
#endif
