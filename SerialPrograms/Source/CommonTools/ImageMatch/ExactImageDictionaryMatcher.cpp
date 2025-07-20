/*  Exact Image Matcher
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cmath>
#include <vector>
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageDiff.h"
#include "ExactImageDictionaryMatcher.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace ImageMatch{



// Generate candidate images to be matched against by translating the input image area
// (`box` on `screen`) around.
// The returned candidate images are scaled to match template shape `dimension`.
// `tolerance`: how much translation variances to produce.
//   e.g. tolerance of 1 means translating the candidate images around so that it can match
//   the template with at most 1 pixel off on the template image. 
std::vector<ImageRGB32> make_image_set(
    const ImageViewRGB32& screen,
    const ImageFloatBox& box,
    size_t width, size_t height,
    size_t tolerance
){
    double num_template_pixels = (double)width * height;
    double num_image_pixels = screen.width() * box.width * screen.height() * box.height;
//    cout << std::sqrt(image / num_template_pixels) << endl;
    // scale: roughly the relative size between the input image and the template.
    // e.g. if the input image is 10 x 6 and template is 5 x 3, then `scale` is 2.
    ptrdiff_t scale = (ptrdiff_t)(std::sqrt(num_image_pixels / num_template_pixels) + 0.5);
    scale = std::max<ptrdiff_t>(scale, 1);

    std::vector<ImageRGB32> ret;
    ptrdiff_t limit = (ptrdiff_t)tolerance;
    for (ptrdiff_t y = -limit; y <= limit; y++){
        for (ptrdiff_t x = -limit; x <= limit; x++){
//            if (x != 0 || y != -4){
//                continue;
//            }

            ret.emplace_back(
                extract_box_reference(screen, box, x * scale, y * scale).scale_to(width, height)
            );
//            cout << "make_image_set(): image = " << ret.back().width() << " x " << ret.back().height() << endl;
//            if (x == 0 && y == 0){
//                ret.back().save("image.png");
//            }
        }
    }
//    cout << "size = " << ret.size() << endl;
    return ret;
}



ExactImageDictionaryMatcher::ExactImageDictionaryMatcher(const WeightedExactImageMatcher::InverseStddevWeight& weight)
    : m_weight(weight)
{}
void ExactImageDictionaryMatcher::add(const std::string& slug, ImageRGB32 image){
    if (!image){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Null image.");
    }
    if (m_width != 0){
        if (image.width() != m_width || image.height() != m_height){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Mismatching dimensions.");
        }
    }else{
        m_width = image.width();
        m_height = image.height();
    }
    auto iter = m_database.find(slug);
    if (iter != m_database.end()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Duplicate slug: " + slug);
    }

    m_database.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(slug),
        std::forward_as_tuple(std::move(image), m_weight)
    );
//    if (slug == "linoone-galar" || slug == "coalossal"){
//        cout << slug << " = " << m_database.find(slug)->second.stats().stddev.sum() << endl;
//    }
}


#if 0
void ExactImageDictionaryMatcher::scale_to_dimensions(ImageRGB32& image) const{
    if (image.width() != m_width || image.height() != m_height){
        image = image.scale_to(m_width, m_height);
    }
    }
}
#endif


double ExactImageDictionaryMatcher::compare(
    const WeightedExactImageMatcher& sprite,
    const std::vector<ImageRGB32>& images
){
//    sprite.m_image.save("sprite.png");
//    images[0].save("image.png");

    double best = 10000;
    for (const ImageRGB32& image : images){
        double rmsd_alpha = sprite.diff(image);
//        cout << rmsd_alpha << endl;
//        if (rmsd_alpha < 0.38){
//            sprite.m_image.save("sprite.png");
//            image.save("image.png");
//        }
        best = std::min(best, rmsd_alpha);
    }
//    cout << best << endl;
    return best;
}

ImageMatchResult ExactImageDictionaryMatcher::match(
    const ImageViewRGB32& image, const ImageFloatBox& box,
    size_t tolerance,
    double alpha_spread
) const{
    ImageMatchResult results;
    if (!image){
        return results;
    }

    // Translate the input image area a bit to careate matching candidates.
    std::vector<ImageRGB32> image_set = make_image_set(image, box, m_width, m_height, tolerance);
    for (const auto& item : m_database){
//        if (item.first != "linoone-galar"){
//            continue;
//        }
        double alpha = compare(item.second, image_set);
        results.add(alpha, item.first);
        results.clear_beyond_spread(alpha_spread);
    }

    return results;
}

ImageMatchResult ExactImageDictionaryMatcher::subset_match(
    const std::vector<std::string>& subset,
    const ImageViewRGB32& image, const ImageFloatBox& box,
    size_t tolerance,
    double alpha_spread
) const{
    ImageMatchResult results;
    if (!image){
        return results;
    }

    // Translate the input image area a bit to careate matching candidates.
    std::vector<ImageRGB32> image_set = make_image_set(image, box,  m_width, m_height, tolerance);
    for (const auto& slug : subset){
        const auto& matcher = image_matcher(slug);
        double alpha = compare(matcher, image_set);
        results.add(alpha, slug);
        results.clear_beyond_spread(alpha_spread);
    }

    return results;
}

ImageViewRGB32 ExactImageDictionaryMatcher::image_template(const std::string& slug) const{
    auto it = m_database.find(slug);
    if (it == m_database.end()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Unknown slug: " + slug);
    }

    return it->second.image_template();
}

const WeightedExactImageMatcher& ExactImageDictionaryMatcher::image_matcher(const std::string& slug) const{
    auto it = m_database.find(slug);
    if (it == m_database.end()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Unknown slug: " + slug);
    }

    return it->second;
}


}
}
