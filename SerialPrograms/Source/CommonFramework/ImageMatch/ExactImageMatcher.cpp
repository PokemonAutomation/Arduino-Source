/*  Exact Image Matcher
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <cmath>
#include "Common/Cpp/Exception.h"
#include "ImageDiff.h"
#include "ExactImageMatcher.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace ImageMatch{




std::vector<QImage> make_image_set(
    const QImage& screen,
    const ImageFloatBox& box,
    const QSize& dimensions,
    size_t tolerance
){
    std::vector<QImage> ret;
    int limit = (int)tolerance;
    for (int y = -limit; y <= limit; y++){
        for (int x = -limit; x <= limit; x++){
            ret.emplace_back(extract_box(screen, box, x, y).scaled(dimensions));
//            if (x == 0 && y == 0){
//                ret.back().save("image.png");
//            }
        }
    }
    return ret;
}


void ExactImageMatcher::add(const std::string& slug, QImage image){
    if (image.isNull()){
        PA_THROW_StringException("Null image.");
    }
//    image = image.scaled(image.width() * 8, image.height() * 8);
//    image = image.scaled(image.width() * 8, image.height() * 8, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    if (m_dimensions.isValid()){
        if (image.size() != m_dimensions){
            PA_THROW_StringException("Mismatching dimensions.");
        }
    }else{
        m_dimensions = image.size();
    }
    auto iter = m_database.find(slug);
    if (iter != m_database.end()){
        PA_THROW_StringException("Duplicate slug: " + slug);
    }

    m_database.emplace(slug, std::move(image));
}


void ExactImageMatcher::scale_to_dimensions(QImage& image) const{
    if (image.size() != m_dimensions){
        image = image.scaled(m_dimensions);
//        image = image.scaled(m_dimensions, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }
}


double ExactImageMatcher::compare(
    const ExactMatchMetadata& sprite,
    QImage image, bool use_alpha_mask
){
    if (use_alpha_mask){
        return sprite.rmsd_ratio_masked_with(image);
    }else{
        return sprite.rmsd_ratio_with(image);
    }

#if 0
    FloatPixel image_brightness = pixel_average(image, sprite.m_image);
    FloatPixel scale = sprite.m_average_pixel / image_brightness;
    if (std::isnan(scale.r)) scale.r = 1.0;
    if (std::isnan(scale.g)) scale.g = 1.0;
    if (std::isnan(scale.b)) scale.b = 1.0;
//    scale.bound(0.5, 2.0);
    scale.bound(0.8, 1.2);

    scale_brightness(image, scale);
    double ret = use_alpha_mask
        ? pixel_RMSD_masked(sprite.m_image, image)
        : pixel_RMSD(sprite.m_image, image);
//    cout << ret << endl;

#if 0
    if (ret < 103){
        cout << pixel_max_possible_RMSD(sprite) << endl;
//        cout << sprite_brightness << endl;
        image_diff_greyscale(sprite, image).save("diff.png");
        image.save("test.png");
    }
#endif

//    return ret;
    return ret / sprite.m_max_possible_RMSD;
#endif
}
double ExactImageMatcher::compare(
    const ExactMatchMetadata& sprite,
    const std::vector<QImage>& images, bool use_alpha_mask
){
    double best = 1000;
    for (const QImage& image : images){
        best = std::min(best, compare(sprite, image, use_alpha_mask));
    }
//    cout << best << endl;
    return best;
}

MatchResult ExactImageMatcher::match(
    const QImage& screen, const ImageFloatBox& box,
    bool use_alpha_mask,
    size_t tolerance,
    double RMSD_ratio_spread
) const{
    if (screen.isNull()){
        return MatchResult();
    }

    std::vector<QImage> image_set = make_image_set(screen, box, m_dimensions, tolerance);

    std::multimap<double, std::string> slugs;

    for (const auto& item : m_database){
//        if (item.first != "butterfree"){
//            continue;
//        }
        slugs.emplace(
            compare(
                item.second,
                image_set, use_alpha_mask
            ),
            item.first
        );
        while (slugs.size() > 1){
            auto best = slugs.begin();
            auto back = slugs.rbegin();
            if (back->first <= best->first + RMSD_ratio_spread){
                break;
            }
            slugs.erase(back->first);
        }
    }

    MatchResult result;
    result.slugs = std::move(slugs);
    return result;
}





}
}
