/*  Box Reorder National Dex
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_Ordering_H
#define PokemonAutomation_PokemonSwSh_Ordering_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonTools/Options/LanguageOCROption.h"
#include "NintendoSwitch/Options/NintendoSwitch_StartInGripMenuOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class BoxReorderNationalDex_Descriptor : public SingleSwitchProgramDescriptor{
public:
    BoxReorderNationalDex_Descriptor();
};



class BoxReorderNationalDex : public SingleSwitchProgramInstance{
public:
    BoxReorderNationalDex();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    StartInGripOrGameOption START_LOCATION;
    OCR::LanguageOCROption LANGUAGE;
    SimpleIntegerOption<uint16_t> POKEMON_COUNT;
    BooleanCheckBoxOption DODGE_SYSTEM_UPDATE_WINDOW;
};


}
}
}
#endif

