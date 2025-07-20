/*  Exact Image Match Preprocessed Data
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonTools_ExactImageMatcher_H
#define PokemonAutomation_CommonTools_ExactImageMatcher_H

#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTools/ImageStats.h"

namespace PokemonAutomation{
namespace ImageMatch{


//  Match images against a template.
//  Before matching, resize the input image to the template shape and scale template brightness to
//  match the input image. Alpha channels as used as masks in matching.
//  No other treatment like tolerating translation or scaling based on stddevs, hence the name "Exact".
class ExactImageMatcher{
    ExactImageMatcher(ExactImageMatcher&&) = default;
    ExactImageMatcher& operator=(ExactImageMatcher&&) = default;
    ExactImageMatcher(const ExactImageMatcher&) = delete;
    void operator=(const ExactImageMatcher&) = delete;

public:
    ExactImageMatcher(const std::string& image_template)
        : ExactImageMatcher(ImageRGB32(image_template))
    {}
    ExactImageMatcher(ImageRGB32 image_template);
    
    const ImageStats& stats() const{ return m_stats; }

    // Resize image to match the shape of the image template, scale the template brightness to match
    // the input image, then compute their RMSD (root mean square deviation).
    // The part of the image template where alpha is 0 is not used to compare with the corresponding
    // part in the input image.
    double rmsd(const ImageViewRGB32& image) const;
    // Resize image to match the shape of the image template, scale the template brightness to match
    // the input image, then compute their RMSD (root mean square deviation).
    // The part of the image template where alpha is 0 is replace with `background` color when comparing
    // against the corresponding part in the input image.
    double rmsd(const ImageViewRGB32& image, Color background) const;
    // Resize image to match the shape of the image template, scale the template brightness to match
    // the input image, then compute their RMSD (root mean square deviation).
    // Alpha channels from both the template and the input image are considered when computing RMSD.
    // If only one of the two has alpha==255 on one pixel, that the deviation on that pixel is the max pixel
    // distance.
    // If both two images have alpha==0 on one pixel, that pixel is ignored.
    double rmsd_masked(const ImageViewRGB32& image) const;

    const ImageRGB32& image_template() const { return m_image; }

private:
    // scale stored image template according to the brightness of `image`, assign
    // the scaled template to `reference`.
    ImageRGB32 scale_template_brightness(const ImageViewRGB32& image) const;

protected:
    ImageRGB32 m_image;
    ImageStats m_stats;
};


// Based on ExactImageMatcher, adds new stddev scaling.
// An image template with higher stddev tends to have lots of detail. RMSD against an input image tends to
// be high because high details will be skewered by stuff like compression artifacts and translational shifts.
// So a template with high stddev tends to give higher RMSD than one with low stddev.
// To compensate for this, when calling WeightedExactImageMatcher::diff...(), it scales the computed RMSD
// based on template stddev, tunable by additional parameters defined in
// WeightedExactImageMatcher::InverseStddevWeight.
class WeightedExactImageMatcher : public ExactImageMatcher{
public:
    // Used to tune RMSD based on stddev of the template
    // Equation:
    // RMSD_scaled = RMSD_original / (template_stddev * `stddev_coefficient` + `offset`)
    // where template_stddev is the sum of the stddev on all three channels (RGB) of the template.
    struct InverseStddevWeight{
        double stddev_coefficient = 0.0;
        double offset = 1.0;
    };

    WeightedExactImageMatcher(ImageRGB32 image_template, const InverseStddevWeight& weight);

    // Like ExactImageMatcher::rmsd(image) but scale based on template stddev.
    double diff(const ImageViewRGB32& image) const;
    // Like ExactImageMatcher::rmsd(image, background) but scale based on template stddev.
    double diff(const ImageViewRGB32& image, Color background) const;
    // Like ExactImageMatcher::rmsd_masked(image) but scale based on template stddev.
    double diff_masked(const ImageViewRGB32& image) const;

public:
    double m_multiplier;
};


}
}
#endif
