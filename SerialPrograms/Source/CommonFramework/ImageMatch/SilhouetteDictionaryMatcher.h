/*  Silhouette Dictionary Matcher
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_SilhouetteDictionaryMatcher_H
#define PokemonAutomation_CommonFramework_SilhouetteDictionaryMatcher_H

#include "Common/Compiler.h"
#include "CommonFramework/ImageTools/FloatPixel.h"
#include "ImageMatchResult.h"
#include "ExactImageMatcher.h"

namespace PokemonAutomation{
    class ImageViewRGB32;
namespace ImageMatch{


class SilhouetteDictionaryMatcher{
public:
    SilhouetteDictionaryMatcher(SilhouetteDictionaryMatcher&&) = default;
    SilhouetteDictionaryMatcher& operator=(SilhouetteDictionaryMatcher&&) = default;
    SilhouetteDictionaryMatcher(const SilhouetteDictionaryMatcher&) = delete;
    void operator=(const SilhouetteDictionaryMatcher&) = delete;

public:
    SilhouetteDictionaryMatcher() = default;
    virtual ~SilhouetteDictionaryMatcher() = default;

    void add(const std::string& slug, ImageRGB32 image);

    ImageMatchResult match(const ImageViewRGB32& image, double alpha_spread) const;


protected:
    virtual ImageRGB32 process_image(const ImageViewRGB32& image, QRgb& background) const = 0;


private:
    std::map<std::string, ExactImageMatcher> m_database;
};


}
}
#endif
