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

    m_database.emplace(slug, trim_image_alpha(image));
}



MatchResult CroppedImageMatcher::match(
    QImage image,
    double RMSD_ratio_spread
) const{
    if (image.isNull()){
        return MatchResult();
    }


    QRgb background = crop_image(image);
//    image.save("test.png");
//    cout << FloatPixel(background) << endl;
    set_alpha_channels(image);



    std::multimap<double, std::string> slugs;
    for (const auto& item : m_database){
//        if (item.first != "solosis"){
//            continue;
//        }

        double rmsd_ratio;
        if (m_use_background){
            rmsd_ratio = item.second.rmsd_ratio_with(image, background);
        }else{
            rmsd_ratio = item.second.rmsd_ratio_masked_with(image);
        }
        slugs.emplace(rmsd_ratio, item.first);

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
