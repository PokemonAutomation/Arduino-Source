/*  Exact Image Matcher
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <cmath>
#include "Common/Cpp/Exception.h"
#include "ImageDiff.h"
#include "ExactImageMatcher.h"

namespace PokemonAutomation{
namespace ImageMatch{



//ExactImageMatcher::ExactImageMatcher(size_t width, size_t height)
//    : m_dimensions(width, height)
//{}

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

    Sprite sprite;
    sprite.average_pixel = pixel_average(image, image);
    sprite.sprite = std::move(image);
    m_database.emplace(
        slug,
        std::move(sprite)
    );
}


void ExactImageMatcher::scale_to_dimensions(QImage& image) const{
    if (image.size() != m_dimensions){
        image = image.scaled(m_dimensions);
//        image = image.scaled(m_dimensions, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }
}


double ExactImageMatcher::compare(
    const QImage& sprite, const FloatPixel& sprite_brightness,
    const QImage& image, const ImageFloatBox& box,
    bool use_alpha_mask,
    int offset_x, int offset_y
) const{
    QImage processed = extract_box(image, box, offset_x, offset_y);
    scale_to_dimensions(processed);
    FloatPixel image_brightness = pixel_average(processed, sprite);
    FloatPixel scale = sprite_brightness / image_brightness;
//        cout << "image  = " << image_brightness << endl;
//        cout << "sprite = " << item.second.average_pixel << endl;
    if (std::isnan(scale.r)) scale.r = 1.0;
    if (std::isnan(scale.g)) scale.g = 1.0;
    if (std::isnan(scale.b)) scale.b = 1.0;
    scale.bound(0.5, 2.0);

    scale_brightness(processed, scale);
    return use_alpha_mask
        ? pixel_RMSD_masked(sprite, processed)
        : pixel_RMSD(sprite, processed);
}
double ExactImageMatcher::compare(
    const QImage& sprite, const FloatPixel& sprite_brightness,
    const QImage& image, const ImageFloatBox& box,
    bool use_alpha_mask,
    int tolerance
) const{
    double best = 1000;
    for (int x = -tolerance; x <= tolerance; x++){
        for (int y = -tolerance; y <= tolerance; y++){
            best = std::min(best, compare(sprite, sprite_brightness, image, box, use_alpha_mask, x, y));
        }
    }
    return best;
}
MatchResult ExactImageMatcher::match(
    const QImage& screen, const ImageFloatBox& box,
    bool use_alpha_mask,
    int tolerance,
    double RMSD_spread
) const{
    if (screen.isNull()){
        return MatchResult();
    }

    const QImage& formatted = screen.format() == QImage::Format_RGB32 || screen.format() == QImage::Format_ARGB32
        ? screen
        : screen.convertToFormat(QImage::Format_RGB32);

    std::multimap<double, std::string> slugs;

    for (const auto& item : m_database){
        slugs.emplace(
            compare(
                item.second.sprite,
                item.second.average_pixel,
                formatted, box,
                use_alpha_mask,
                tolerance
            ),
            item.first
        );
        while (slugs.size() > 1){
            auto best = slugs.begin();
            auto back = slugs.rbegin();
            if (back->first <= best->first + RMSD_spread){
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
