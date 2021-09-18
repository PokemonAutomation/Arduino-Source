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
#include "ImageMatchMetadata.h"

namespace PokemonAutomation{
namespace ImageMatch{


class ExactImageMatcher{
public:
    ExactImageMatcher() = default;
    void add(const std::string& slug, QImage image);

    QSize dimensions() const{ return m_dimensions; }

    void scale_to_dimensions(QImage& image) const;

    MatchResult match(
        const QImage& screen, const ImageFloatBox& box,
        bool use_alpha_mask,
        size_t tolerance = 2,
        double RMSD_ratio_spread = 0.03
    ) const;


private:
    static double compare(
        const ExactMatchMetadata& sprite,
        QImage image, bool use_alpha_mask
    );
    static double compare(
        const ExactMatchMetadata& sprite,
        const std::vector<QImage>& images, bool use_alpha_mask
    );


private:
    QSize m_dimensions;
    std::map<std::string, ExactMatchMetadata> m_database;
};



}
}
#endif
