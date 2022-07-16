/*  Cropped Image Dictionary Matcher
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_CroppedImageDictionaryMatcher_H
#define PokemonAutomation_CommonFramework_CroppedImageDictionaryMatcher_H

#include <QImage>
#include "Common/Compiler.h"
#include "CommonFramework/ImageTools/FloatPixel.h"
#include "ImageMatchResult.h"
#include "ExactImageMatcher.h"

namespace PokemonAutomation{
    class ImageViewRGB32;
namespace ImageMatch{


class CroppedImageDictionaryMatcher{
public:
    CroppedImageDictionaryMatcher(const WeightedExactImageMatcher::InverseStddevWeight& weight);
    virtual ~CroppedImageDictionaryMatcher() = default;

    void add(const std::string& slug, QImage image);

    ImageMatchResult match(const ImageViewRGB32& image, double alpha_spread) const;


protected:
    virtual QRgb crop_image(QImage& image) const{ return 0; }
    virtual void crop_sprite(QImage& image, QRgb background) const{}
    virtual void set_alpha_channels(QImage& image) const{}


private:
    WeightedExactImageMatcher::InverseStddevWeight m_weight;
    std::map<std::string, WeightedExactImageMatcher> m_database;
};


}
}
#endif
