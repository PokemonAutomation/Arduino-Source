/*  Exact Image Matcher
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_ExactImageMatcher_H
#define PokemonAutomation_CommonFramework_ExactImageMatcher_H

#include <string>
#include <map>
#include <QImage>
#include "CommonFramework/Tools/Logger.h"
#include "CommonFramework/Inference/ImageTools.h"
#include "ImageMatchResult.h"

namespace PokemonAutomation{
namespace ImageMatch{


class ExactImageMatcher{
public:
    ExactImageMatcher() = default;
//    ExactImageMatcher(size_t width, size_t height);
    void add(const std::string& slug, QImage image);

    QSize dimensions() const{ return m_dimensions; }

    void scale_to_dimensions(QImage& image) const;

#if 0
    MatchResult match(
        const QImage& image,
        bool use_alpha_mask,
        double RMSD_spread = 30
    ) const;
#endif

    MatchResult match(
        const QImage& screen, const ImageFloatBox& box,
        bool use_alpha_mask,
        int tolerance = 2,
        double RMSD_spread = 30
    ) const;


private:
    double compare(
        const QImage& sprite, const FloatPixel& sprite_brightness,
        const QImage& image, const ImageFloatBox& box,
        bool use_alpha_mask,
        int offset_x, int offset_y
    ) const;
    double compare(
        const QImage& sprite, const FloatPixel& sprite_brightness,
        const QImage& image, const ImageFloatBox& box,
        bool use_alpha_mask,
        int tolerance
    ) const;


private:
    struct Sprite{
        QImage sprite;
        FloatPixel average_pixel;
    };

    QSize m_dimensions;
    std::map<std::string, Sprite> m_database;
};



}
}
#endif
