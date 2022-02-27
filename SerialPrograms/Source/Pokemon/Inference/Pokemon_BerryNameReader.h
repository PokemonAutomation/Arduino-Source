/*  Berry Name Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Pokemon_BerryNameReader_H
#define PokemonAutomation_Pokemon_BerryNameReader_H

#include <QImage>
#include "CommonFramework/Logging/LoggerQt.h"
#include "CommonFramework/OCR/OCR_SmallDictionaryMatcher.h"

namespace PokemonAutomation{
namespace Pokemon{


class BerryNameReader : public OCR::SmallDictionaryMatcher{
    static constexpr double MAX_LOG10P = -1.40;

public:
    BerryNameReader();

    static BerryNameReader& instance();

    OCR::StringMatchResult read_substring(
        LoggerQt& logger,
        Language language,
        const QImage& image
    ) const;
};


}
}
#endif
