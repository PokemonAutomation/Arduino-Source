/*  IV Checker Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Pokemon_IVCheckerReader_H
#define PokemonAutomation_Pokemon_IVCheckerReader_H

#include "Pokemon/Pokemon_IVChecker.h"
#include "CommonFramework/OCR/OCR_SmallDictionaryMatcher.h"

namespace PokemonAutomation{
namespace Pokemon{



class IVCheckerReader : public OCR::SmallDictionaryMatcher{
public:
    struct Results{
        IVCheckerValue hp       = IVCheckerValue::UnableToDetect;
        IVCheckerValue attack   = IVCheckerValue::UnableToDetect;
        IVCheckerValue defense  = IVCheckerValue::UnableToDetect;
        IVCheckerValue spatk    = IVCheckerValue::UnableToDetect;
        IVCheckerValue spdef    = IVCheckerValue::UnableToDetect;
        IVCheckerValue speed    = IVCheckerValue::UnableToDetect;
    };

    static const IVCheckerReader& instance();

private:
    IVCheckerReader();
};



}
}
#endif
