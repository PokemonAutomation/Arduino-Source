/*  Read Summary Screen
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonHome_ReadSummaryScreen_H
#define PokemonAutomation_PokemonHome_ReadSummaryScreen_H

#include "CommonTools/Options/LanguageOCROption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{

class ReadSummaryScreen_Descriptor : public SingleSwitchProgramDescriptor{
public:
    ReadSummaryScreen_Descriptor();
};

class ReadSummaryScreen : public SingleSwitchProgramInstance{
public:
    ReadSummaryScreen();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    OCR::LanguageOCROption OT_NAME_LANGUAGE;
    OCR::LanguageOCROption HOME_LANGUAGE;
};

}
}
}
#endif
