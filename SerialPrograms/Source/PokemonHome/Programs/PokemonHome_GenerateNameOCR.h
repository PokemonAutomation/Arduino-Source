/*  Pokemon Home Generate Name OCR
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonHome_GenerateNameOCR_H
#define PokemonAutomation_PokemonHome_GenerateNameOCR_H

#include "Common/Cpp/Options/TimeDurationOption.h"
#include "CommonTools/Options/LanguageOCROption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{


class GenerateNameOCRData_Descriptor : public SingleSwitchProgramDescriptor{
public:
    GenerateNameOCRData_Descriptor();
};



class GenerateNameOCRData : public SingleSwitchProgramInstance{

public:
    GenerateNameOCRData();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    OCR::LanguageOCROption LANGUAGE;
    MillisecondsOption DELAY0;
};



}
}
}
#endif
