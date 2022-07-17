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
namespace ImageMatch{


class CroppedImageDictionaryMatcher{
public:
    CroppedImageDictionaryMatcher(const WeightedExactImageMatcher::InverseStddevWeight& weight);
    virtual ~CroppedImageDictionaryMatcher() = default;

    void add(const std::string& slug, const ImageViewRGB32& image);

    ImageMatchResult match(const ImageViewRGB32& image, double alpha_spread) const;


protected:
    virtual ImageRGB32 process_image(const ImageViewRGB32& image, QRgb& background) const = 0;


private:
    WeightedExactImageMatcher::InverseStddevWeight m_weight;
    std::map<std::string, WeightedExactImageMatcher> m_database;
};


}
}
#endif
