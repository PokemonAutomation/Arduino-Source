/*  Box Reorder National Dex
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
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class BoxReorderNationalDex_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    BoxReorderNationalDex_Descriptor();
};



class BoxReorderNationalDex : public SingleSwitchProgramInstance{
public:
    BoxReorderNationalDex(const BoxReorderNationalDex_Descriptor& descriptor);

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

