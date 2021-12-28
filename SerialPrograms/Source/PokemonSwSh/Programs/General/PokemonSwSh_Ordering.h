/*  Ordering
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_Ordering_H
#define PokemonAutomation_PokemonSwSh_Ordering_H

#include "CommonFramework/Options/BooleanCheckBoxOption.h"
#include "CommonFramework/Options/SimpleIntegerOption.h"
#include "NintendoSwitch/Options/StartInGripMenuOption.h"
#include "CommonFramework/OCR/OCR_LanguageOptionOCR.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class Ordering_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    Ordering_Descriptor();
};



class Ordering : public SingleSwitchProgramInstance{
public:
    Ordering(const Ordering_Descriptor& descriptor);

    virtual void program(SingleSwitchProgramEnvironment& env) override;

private:
    StartInGripOrGameOption START_IN_GRIP_MENU;
    OCR::LanguageOCR LANGUAGE;
    SimpleIntegerOption<uint16_t> POKEMON_COUNT;
    BooleanCheckBoxOption DODGE_SYSTEM_UPDATE_WINDOW;
};


}
}
}
#endif

