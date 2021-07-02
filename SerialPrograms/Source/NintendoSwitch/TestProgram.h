/*  Test Program
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_TestProgram_H
#define PokemonAutomation_PokemonSwSh_TestProgram_H

#include "CommonFramework/Options/EnumDropdown.h"
#include "CommonFramework/Options/StringSelect.h"
#include "CommonFramework/Options/LanguageOCR.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"
#include "PokemonSwSh/Inference/PokemonSwSh_IVCheckerReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{

using namespace PokemonSwSh;

class TestProgram_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    TestProgram_Descriptor();
};


class TestProgram : public SingleSwitchProgramInstance{
public:
    TestProgram(const TestProgram_Descriptor& descriptor);

    virtual void program(SingleSwitchProgramEnvironment& env) override;

private:
    IVCheckerReader m_iv_checker_reader;
    LanguageOCR LANGUAGE;
    EnumDropdown DROPDOWN;
    StringSelect STRING_SELECT;
};



}
}
#endif

