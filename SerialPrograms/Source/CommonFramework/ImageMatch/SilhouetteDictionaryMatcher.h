/*  Silhouette Dictionary Matcher
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_SilhouetteDictionaryMatcher_H
#define PokemonAutomation_CommonFramework_SilhouetteDictionaryMatcher_H

#include <QImage>
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

    void add(const std::string& slug, QImage image);

    ImageMatchResult match(const ImageViewRGB32& image, double alpha_spread) const;


protected:
    virtual QRgb crop_image(QImage& image) const{ return 0; }
    virtual void crop_sprite(QImage& image, QRgb background) const{}
    virtual void set_alpha_channels(QImage& image) const{}


private:
    std::map<std::string, ExactImageMatcher> m_database;
};


}
}
#endif
