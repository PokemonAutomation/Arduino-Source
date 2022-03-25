/*  Exact Image Dictionary Matcher
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_ExactImageDictionaryMatcher_H
#define PokemonAutomation_CommonFramework_ExactImageDictionaryMatcher_H

#include <string>
#include <map>
#include "CommonFramework/Logging/LoggerQt.h"
#include "ImageMatchResult.h"
#include "ExactImageMatcher.h"

namespace PokemonAutomation{
    struct ImageFloatBox;
namespace ImageMatch{


class ExactImageDictionaryMatcher{
public:
    ExactImageDictionaryMatcher(const WeightedExactImageMatcher::InverseStddevWeight& weight);
    void add(const std::string& slug, QImage image);

    QSize dimensions() const{ return m_dimensions; }

    void scale_to_dimensions(QImage& image) const;

    ImageMatchResult match(
        const ConstImageRef& image, const ImageFloatBox& box,
        size_t tolerance,
        double alpha_spread
    ) const;


private:
    static double compare(
        const WeightedExactImageMatcher& sprite,
        const std::vector<QImage>& images
    );


private:
    WeightedExactImageMatcher::InverseStddevWeight m_weight;
    QSize m_dimensions;
    std::map<std::string, WeightedExactImageMatcher> m_database;
};



}
}
#endif
