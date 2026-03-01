/*  Sub-Object Template Matcher
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *      Suppose we have an object with a distinctive sub-feature.
 *
 *  We want to find this object in an image. But instead of searching for the
 *  entire object as a whole, we search for that distinctive sub-feature.
 *
 *  Once we've found that sub-feature and have it's bounding box, we want to
 *  verify that the rest of the object matches the template.
 *
 */

#ifndef PokemonAutomation_CommonTools_SubObjectTemplateMatcher_H
#define PokemonAutomation_CommonTools_SubObjectTemplateMatcher_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTypes/BinaryImage.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "ExactImageMatcher.h"

namespace PokemonAutomation{
namespace ImageMatch{


// A base-class image matcher that uses the match of a sub-object feature inside an object to
// locate the object and then checks the similarity between the object and the template.
//
// EXAMPLE: Detecting Pikachu Profile Picture by Red Cheek: we first match the red cheek, then
// use the relative location of the cheek w.r.t the whole pikachu picture to locate a potential
// pikachu candidate to match against a template pikachu picture.
//
// USAGE:
// ======
// 1. Create a derived class:
//    class FlagMatcher : public SubObjectTemplateMatcher {
//        FlagMatcher()
//            : SubObjectTemplateMatcher("PokemonLA/Flag-Template.png", 100)
//        {
//            // Load template, find the left/right bar in it, set as sub-object
//            std::vector<WaterfillObject> objects = find_objects_inplace(...);
//            set_subobject(objects[0]);  // Set part of the template image is the sub-object
//        }
//    };
//
// 2. At runtime, use it to check candidates:
//    FlagMatcher matcher(true);  // Looking for left bar
//    ImagePixelBox object_box;
//    if (matcher.matches(object_box, image, candidate_waterfill_object)){
//        // Found a flag! object_box contains the full flag bounding box
//    }
class SubObjectTemplateMatcher{
protected:
    using WaterfillObject = Kernels::Waterfill::WaterfillObject;

public:
    virtual ~SubObjectTemplateMatcher() = default;

    // Constructor: Load template image and set matching threshold
    // - path: Relative path to template image (e.g., "PokemonLA/Flag-Template.png")
    // - max_rmsd: Maximum RMSD (root mean square difference) to consider a match
    //   Lower values = stricter matching. Typical range: 50-150
    SubObjectTemplateMatcher(const char* path, double max_rmsd);

    // Constructor with background replacement support
    // Use this when the object has transparent or variable background pixels
    // - path: Relative path to template image (e.g., "PokemonLA/Flag-Template.png")
    // - background_replacement: Color to replace background pixels with before matching
    // - max_rmsd: Maximum RMSD (root mean square difference) to consider a match
    //   Lower values = stricter matching. Typical range: 50-150
    SubObjectTemplateMatcher(const char* path, Color background_replacement, double max_rmsd);

    // Calculate where the full object should be, given a detected sub-feature location
    // - width, height: Dimensions of the full image being searched
    // - subobject_in_image: Bounding box of the detected sub-feature w.r.t the full image.
    // Returns: Bounding box where the full object should be located w.r.t the full image.
    ImagePixelBox object_from_subobject(
        size_t width, size_t height,
        const ImagePixelBox& subobject_in_image
    ) const;

    // Compute RMSD between candidate object and template. RMSD = sqrt(mean((pixel_differences)^2))
    // - object_box: [OUTPUT] Calculated bounding box of full object w.r.t to the input image
    // - image: Input image to search in
    // - subobject_in_image: Detected sub-feature location w.r.t. the image
    // Returns: RMSD value (lower is better, 0 = perfect match)
    double rmsd(
        ImagePixelBox& object_box,
        const ImageViewRGB32& image, const ImagePixelBox& subobject_in_image
    ) const;

    // Compute RMSD with background pixel replacement. RMSD = sqrt(mean((pixel_differences)^2))
    // Before computing RMSD, replaces background pixels with `m_background_replacement` color
    // - object_box: [OUTPUT] Calculated bounding box of full object w.r.t to the input image
    // - image: Input image to search in
    // - binary_image: Binary mask of the image where 0 = background, 1 = foreground
    // - subobject_in_image: Detected sub-feature location w.r.t. the image
    double rmsd_with_background_replace(
        ImagePixelBox& object_box,
        const ImageViewRGB32& image, const PackedBinaryMatrix& binary_image,
        const ImagePixelBox& subobject_in_image
    ) const;

    // Given a found sub-feature as the waterfill object, check if the whole object matches the
    // template.
    // - object_box: [OUTPUT] If match succeeds, contains full object bounding box
    // - image: Input image to search in
    // - subobject_in_image: Candidate sub-feature as a waterfill object
    // Returns: true if it matches the template
    virtual bool matches(
        ImagePixelBox& object_box,
        const ImageViewRGB32& image,
        const WaterfillObject& subobject_in_image
    ) const;

    // Given a found sub-feature as the waterfill object, check if the whole object matches the
    // template.
    // Before computing RMSD, replaces background pixels with `m_background_replacement` color
    // - object_box: [OUTPUT] If match succeeds, contains full object bounding box
    // - image: Input image to search in
    // - binary_image: Binary mask of the image where 0 = background, 1 = foreground
    // - subobject_in_image: Candidate sub-feature as a waterfill object
    // Returns: true if it matches the template
    virtual bool matches_with_background_replace(
        ImagePixelBox& object_box,
        const ImageViewRGB32& image, const PackedBinaryMatrix& binary_image,
        const WaterfillObject& subobject_in_image
    ) const;


protected:
    // Optional custom image validation hook called before rmsd checks.
    // Override in derived class to add custom checks before matching.
    // This will be called in all rmsd...() functions and therefore matches...() functions.
    virtual bool check_image(const ImageViewRGB32& image) const{ return true; };

    // Set which part of the template image is the sub-object to search for.
    // MUST be called in derived class constructor to configure the matcher.
    // - subobject_in_object: waterfill object to get the sub-object feature
    //   within the template image.
    void set_subobject(const WaterfillObject& subobject_in_object);

    // Check if candidate sub-object has the expected aspect ratio
    // Compares (width/height) of candidate against (width/height) of sub-object in template
    // Tolerance: Must be within [0.80, 1.25] of expected ratio (configurable via member vars)
    // Returns: true if aspect ratio is acceptable
    bool check_aspect_ratio(size_t candidate_width, size_t candidate_height) const;

    // Check if candidate sub-object has the expected area ratio
    // Area ratio = (object area) / (bounding box area)
    // Useful for filtering by "how much of the bounding box is filled"
    // Returns: true if area ratio is within acceptable range
    bool check_area_ratio(double candidate_area_ratio) const;

protected:
    std::string m_path;                   // Full path to template image file
    Color m_background_replacement;       // Color to replace background with (if using background replacement)
    double m_max_rmsd;                    // Maximum RMSD to consider a match (lower = stricter)

    double m_aspect_ratio_lower = 0.80;   // aspect ratio comparison lower bound to match sub-object
    double m_aspect_ratio_upper = 1.25;   // aspect ratio comparison upper bound to match sub-object
    double m_area_ratio_lower = 0.80;     // area ratio comparison lower bound to match sub-object
    double m_area_ratio_upper = 1.25;     // area ratio comparison upper bound to match sub-object

    // Template image matcher used to compute RMSD comparison against the template full-object.
    ExactImageMatcher m_matcher;

    // sub-object bounding box w.r.t the full object, in pixel units, set by set_subobject()
    ImagePixelBox m_subobject_in_object_p;
    // sub-object bounding box w.r.t the full object, in normalized float units, set by set_subobject()
    ImageFloatBox m_subobject_in_object_f;

    // Area ratio of the sub-object, set by set_subobject()
    // Area ratio is defined as the pixels of the sub-object over the total pixels of the bounding box of the
    // sub-object.
    double m_subobject_area_ratio;
};







}
}
#endif
