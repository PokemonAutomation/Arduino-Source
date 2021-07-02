/*  Pokemon Home Generate Name OCR
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonHome_GenerateNameOCR_H
#define PokemonAutomation_PokemonHome_GenerateNameOCR_H

#include "CommonFramework/Options/LanguageOCR.h"
#include "NintendoSwitch/Options/TimeExpression.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"
#include "Pokemon/Pokemon_NameReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{


class GenerateNameOCRData_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    GenerateNameOCRData_Descriptor();
};



class GenerateNameOCRData : public SingleSwitchProgramInstance{

public:
    GenerateNameOCRData(const GenerateNameOCRData_Descriptor& descriptor);

    virtual void program(SingleSwitchProgramEnvironment& env) override;

private:
    Pokemon::PokemonNameReader m_reader;

    LanguageOCR LANGUAGE;
    TimeExpression<uint16_t> DELAY;
};



}
}
}
#endif
