/*  Generate Pokemon Sprite Data (Pokedex)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_GeneratePokedexSprites_H
#define PokemonAutomation_PokemonSwSh_GeneratePokedexSprites_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonTools/Options/LanguageOCROption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class GeneratePokedexSprites_Descriptor : public SingleSwitchProgramDescriptor{
public:
    GeneratePokedexSprites_Descriptor();
};



class GeneratePokedexSprites : public SingleSwitchProgramInstance{
public:
    GeneratePokedexSprites();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;


private:
    OCR::LanguageOCROption LANGUAGE;
    SimpleIntegerOption<uint16_t> HORIZONTAL_FRAMES;
    SimpleIntegerOption<uint16_t> ANIMATION_FRAMES;
};



}
}
}
#endif
