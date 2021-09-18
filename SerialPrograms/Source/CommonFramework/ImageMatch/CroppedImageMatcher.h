/*  Cropped Image Matcher
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_CroppedImageMatcher_H
#define PokemonAutomation_CommonFramework_CroppedImageMatcher_H

#include <QImage>
#include "CommonFramework/ImageTools/FloatPixel.h"
#include "ImageMatchResult.h"
#include "ImageMatchMetadata.h"

namespace PokemonAutomation{
namespace ImageMatch{


class CroppedImageMatcher{
public:
    CroppedImageMatcher(bool use_background);
    virtual ~CroppedImageMatcher() = default;

    void add(const std::string& slug, QImage image);

    MatchResult match(QImage image, double RMSD_ratio_spread = 0.03) const;


protected:
    virtual QRgb crop_image(QImage& image) const{ return 0; }
    virtual void crop_sprite(QImage& image, QRgb background) const{}
    virtual void set_alpha_channels(QImage& image) const{}


private:
    bool m_use_background;
    std::map<std::string, ExactMatchMetadata> m_database;
};


}
}
#endif
