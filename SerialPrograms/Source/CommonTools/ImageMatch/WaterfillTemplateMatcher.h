/*  Waterfill Template Matcher
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonTools_WaterfillTemplateMatcher_H
#define PokemonAutomation_CommonTools_WaterfillTemplateMatcher_H

#include <memory>
#include "Common/Cpp/Color.h"
#include "Common/Cpp/ImageResolution.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/ImageMatch/ExactImageMatcher.h"

namespace PokemonAutomation{
namespace ImageMatch{


// Used to match an image or a waterfill object against a template object.
class WaterfillTemplateMatcher{
protected:
    using WaterfillObject = Kernels::Waterfill::WaterfillObject;

public:
    WaterfillTemplateMatcher(WaterfillTemplateMatcher&&) = default;
    virtual ~WaterfillTemplateMatcher() = default;


public:
    // Load a template image from disk, use waterfill to find the biggest object in the template image that
    // matches the color range of min_color and max_color. 
    // min_area is the minimum number of pixels required for the found object in the template.
    // 
    // The portion of the image holding the biggest object will get cropped and saved as the actual template
    // image.
    // So if someone changes the template image by padding it to make it larger, as long as the padded color
    // does not fall into [min_color and max_color] range, it will not affect the template matching outcome
    // in any way.
    //
    // Throw FileException when there is no object meeting the requirement in the template.
    // If there are more than one objects meeting the requirement in the template, pick the one
    // with the largest area (largest number of pixels).
    WaterfillTemplateMatcher(
        const char* path,
        Color min_color, Color max_color,
        size_t min_area
    );

    //  Compute RMSD of the current image against the template as-is, using `ExactImageMatcher`.
    // `ExactImageMatcher` will resize the image to match template size and scale template brightness to match the image
    //  before computing RMSD.
    //  The part of the image template where alpha is 0 is not used to compare with the corresponding part in the input image.
    //  In case the image is invalid, return a large value.
    //  It also calls the virtual function `check_image(input_resolution, image)` on the image, where `input_resolution` is
    //  the full screen resolution where the image is from. If the function returns false, then return a large value. 
    double rmsd(Resolution input_resolution, const ImageViewRGB32& image) const;

    //  Compute RMSD of the object on the image against the template.
    //  The input `cropped_image` is already cropped from a full image using the bounding box of the input waterfill `object`.
    //  This cropped image is compared against the template as-is.
    //  The waterfill object's aspect ratio and area ratio are checked against template's. Return a large value 
    //  if the check fails.
    //  See `double rmsd(Resolution input_resolution, const ImageViewRGB32& image) const` on the details of comparing the
    //  image against the template.
    virtual double rmsd_precropped(
        Resolution input_resolution,
        const ImageViewRGB32& cropped_image,
        const WaterfillObject& object
    ) const;

    //  Compute RMSD of the object on the image against the template.
    //  It will crop the original image using the bounding box of the waterfill object, then compare the cropped
    //  image against the template as-is.
    //  The waterfill object's aspect ratio and area ratio are checked against template's. Return a large value 
    //  if the check fails.
    //  See `double rmsd(Resolution input_resolution, const ImageViewRGB32& image) const` on the details of comparing the
    //  image against the template.
    virtual double rmsd_original(
        Resolution input_resolution,
        const ImageViewRGB32& original_image,
        const WaterfillObject& object
    ) const;

    //  Return the image template mesh
    const ImageRGB32& image_template() const { return m_matcher->image_template(); }

protected:
    // This function is called inside each rmsd...() function before the actual RMSD computation.
    // Derived classes can override this function to offer additional checks before computing RMSD.
    // rmsd...() functions will proceed to RMSD computation if this function returns true. If false
    // returns a large value.
    // input_resolution: resolution of the full screen where the input image is from. This is useful for checks on
    // input image pixel count or minimum size.
    virtual bool check_image(Resolution input_resolution, const ImageViewRGB32& image) const{ return true; };
    bool check_aspect_ratio(size_t candidate_width, size_t candidate_height) const;
    bool check_area_ratio(double candidate_area_ratio) const;

protected:
    // Below are thresholds of aspect ratio and area ratio to reject a candidate.
    // They are ususally set by the derived class of `WaterfillTemplateMatcher`.
    double m_aspect_ratio_lower = 0.80;
    double m_aspect_ratio_upper = 1.25;
    double m_area_ratio_lower = 0.80;
    double m_area_ratio_upper = 1.25;

    std::unique_ptr<ExactImageMatcher> m_matcher;
    double m_area_ratio;
};



}
}
#endif
