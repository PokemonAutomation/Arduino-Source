/*  Summary Reader Tester
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_SummaryReaderTester_H
#define PokemonAutomation_PokemonBDSP_SummaryReaderTester_H

#include "CommonTools/Options/LanguageOCROption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class SummaryReaderTester_Descriptor : public SingleSwitchProgramDescriptor{
public:
    SummaryReaderTester_Descriptor();
};


class SummaryReaderTester : public SingleSwitchProgramInstance{
public:
    SummaryReaderTester();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    OCR::LanguageOCROption LANGUAGE;
};


}
}
}
#endif
