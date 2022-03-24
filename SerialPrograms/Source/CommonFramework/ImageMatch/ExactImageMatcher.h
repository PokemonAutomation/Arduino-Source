/*  Exact Image Match Preprocessed Data
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_ExactImageMatcher_H
#define PokemonAutomation_ExactImageMatcher_H

#include <QImage>
#include "CommonFramework/ImageTools/FloatPixel.h"
#include "CommonFramework/ImageTools/ImageStats.h"

namespace PokemonAutomation{
namespace ImageMatch{


//  Matching with brightness scaling only.
class ExactImageMatcher{
public:
    ExactImageMatcher(QImage image);

    const ImageStats& stats() const{ return m_stats; }

    double rmsd(const ConstImageRef& image) const;
    double rmsd(const ConstImageRef& image, QRgb background) const;
    double rmsd_masked(const ConstImageRef& image) const;

private:
    void process_images(QImage& reference, const ConstImageRef& image) const;

public:
    QImage m_image;
    ImageStats m_stats;
};


//  Matching with custom weight.
class WeightedExactImageMatcher : public ExactImageMatcher{
public:
    struct InverseStddevWeight{
        double stddev_coefficient;
        double offset;
    };

    WeightedExactImageMatcher(QImage image, const InverseStddevWeight& weight);

    double diff(const ConstImageRef& image) const;
    double diff(const ConstImageRef& image, QRgb background) const;
    double diff_masked(const ConstImageRef& image) const;

public:
    double m_multiplier;
};


}
}
#endif
