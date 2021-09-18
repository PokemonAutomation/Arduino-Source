/*  Image Match Preprocessed Data
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_ImageMatchMetaData_H
#define PokemonAutomation_ImageMatchMetaData_H

#include <QImage>
#include "CommonFramework/ImageTools/FloatPixel.h"

namespace PokemonAutomation{
namespace ImageMatch{


class ExactMatchMetadata{
public:
    ExactMatchMetadata(QImage image);

    double rmsd_ratio_with(QImage image) const;
    double rmsd_ratio_with(QImage image, QRgb background) const;
    double rmsd_ratio_masked_with(QImage image) const;

public:
    QImage m_image;
    FloatPixel m_average_pixel;
    double m_max_possible_RMSD;
};



}
}
#endif
