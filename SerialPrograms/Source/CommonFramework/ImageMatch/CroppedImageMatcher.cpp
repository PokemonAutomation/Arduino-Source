/*  Cropped Image Matcher
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <cmath>
#include "Common/Cpp/Exception.h"
#include "ImageCropper.h"
#include "ImageDiff.h"
#include "CroppedImageMatcher.h"

#include "CommonFramework/Inference/ImageTools.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace ImageMatch{



CroppedImageMatcher::CroppedImageMatcher(bool use_background)
    : m_use_background(use_background)
{}
void CroppedImageMatcher::add(const std::string& slug, QImage image){
    if (image.isNull()){
        PA_THROW_StringException("Null image.");
    }
    auto iter = m_database.find(slug);
    if (iter != m_database.end()){
        PA_THROW_StringException("Duplicate slug: " + slug);
    }

    image = trim_image_alpha(image);

    Sprite sprite;
    sprite.average_pixel = pixel_average(image, image);
    sprite.sprite = std::move(image);
    m_database.emplace(
        slug,
        std::move(sprite)
    );
}



MatchResult CroppedImageMatcher::match(
    QImage image,
    double RMSD_spread
) const{
    if (image.isNull()){
        return MatchResult();
    }

    if (image.format() != QImage::Format_RGB32 && image.format() != QImage::Format_ARGB32){
        image = image.convertToFormat(QImage::Format_RGB32);
    }

    QRgb background = crop_image(image);

    std::multimap<double, std::string> slugs;
    for (const auto& item : m_database){
//        if (item.first != "porygon"){
//            continue;
//        }

        QImage processed_sprite = item.second.sprite;
        crop_sprite(processed_sprite, background);

        //  Prepare Image
        QImage processed_image = image.scaled(processed_sprite.size());
        set_alpha_channels(processed_image);

        //  Brightness scaling
        FloatPixel image_brightness = pixel_average(processed_image, processed_sprite);
//        cout << image_brightness << endl;
//        cout << item.second.average_pixel << endl;
        FloatPixel scale = image_brightness / item.second.average_pixel;
//        cout << "image  = " << image_brightness << endl;
//        cout << "sprite = " << item.second.average_pixel << endl;
        if (std::isnan(scale.r)) scale.r = 1.0;
        if (std::isnan(scale.g)) scale.g = 1.0;
        if (std::isnan(scale.b)) scale.b = 1.0;
        scale.bound(0.5, 2.0);

        scale_brightness(processed_sprite, scale);

//        cout << processed_sprite.width() << " x " << processed_sprite.height() << endl;
//        cout << processed_image.width() << " x " << processed_image.height() << endl;

//        formatted = scale_brightness(formatted, scale);
//        formatted.save("formatted.png");

//        image_diff_greyscale(item.second.sprite, formatted).save("diff.png");

//        processed_sprite.save("sprite.png");
//        processed_image.save("image.png");

        double rmsd;
        if (m_use_background){
//            QRgb scaled_background = (FloatPixel(background) / scale).round();
            rmsd = pixel_RMSD(processed_sprite, processed_image, background);
        }else{
            rmsd = pixel_RMSD_masked(processed_sprite, processed_image);
        }

        slugs.emplace(rmsd, item.first);

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
