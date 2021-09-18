/*  Pokemon Sprite Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_PokemonSpriteReader_H
#define PokemonAutomation_PokemonSwSh_PokemonSpriteReader_H

#include <QImage>
#include "CommonFramework/Tools/Logger.h"
#include "CommonFramework/ImageMatch/ImageMatchResult.h"
#include "CommonFramework/ImageMatch/ExactImageMatcher.h"
#include "CommonFramework/ImageMatch/CroppedImageMatcher.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class PokemonSpriteMatcherExact : public ImageMatch::ExactImageMatcher{
public:
    PokemonSpriteMatcherExact(const std::set<std::string>* subset);
};

//  Used by Max Lair when there's an item blocking the right side.
class PokemonLeftSpriteMatcherExact : public ImageMatch::ExactImageMatcher{
public:
    PokemonLeftSpriteMatcherExact(const std::set<std::string>* subset);
};



class PokemonSpriteMatcherCropped : public ImageMatch::CroppedImageMatcher{
public:
    PokemonSpriteMatcherCropped(const std::set<std::string>* subset, double min_euclidean_distance = 100);

private:
    virtual QRgb crop_image(QImage& image) const override;
    virtual void crop_sprite(QImage& image, QRgb background) const override;

private:
    double m_min_euclidean_distance_squared;
};





}
}
}
#endif
