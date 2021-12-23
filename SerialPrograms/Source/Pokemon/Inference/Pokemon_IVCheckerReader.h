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
        IVCheckerValue hp;
        IVCheckerValue attack;
        IVCheckerValue defense;
        IVCheckerValue spatk;
        IVCheckerValue spdef;
        IVCheckerValue speed;
    };

    static const IVCheckerReader& instance();

private:
    IVCheckerReader();
};



}
}
#endif
