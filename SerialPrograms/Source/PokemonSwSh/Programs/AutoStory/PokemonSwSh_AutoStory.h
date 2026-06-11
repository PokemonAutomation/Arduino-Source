/*  Auto Story
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_AutoStory_H
#define PokemonAutomation_PokemonSwSh_AutoStory_H

#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "CommonTools/Options/LanguageOCROption.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonSwSh/Options/PokemonSwSh_BallSelectOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

using namespace Pokemon;


class AutoStory_Descriptor : public SingleSwitchProgramDescriptor{
public:
    AutoStory_Descriptor();
};

class AutoStory : public SingleSwitchProgramInstance{
public:
    AutoStory();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    virtual void save_from_overworld(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    virtual void mash_until_black_screen(SingleSwitchProgramEnvironment& env, ProControllerContext& context, Button button = BUTTON_B);
    virtual void mash_until_black_screen_multi(SingleSwitchProgramEnvironment& env, ProControllerContext& context, int8_t cycle, Button button = BUTTON_B);
    virtual void run_battle(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    virtual void run_catch(SingleSwitchProgramEnvironment& env, ProControllerContext& context, bool use_pokeball = true);
    virtual void overworld_navigation(SingleSwitchProgramEnvironment& env, ProControllerContext& context, double x, double y, uint16_t seconds_timeout, bool is_catch = false);

    virtual void segment_00(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    virtual void segment_01(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    virtual void segment_02(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    virtual void segment_03(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    virtual void segment_04(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    virtual void segment_05(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    virtual void segment_06(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

    enum class StarterPokemon{
        GROOKEY,
        SCORBUNNY,
        SOBBLE
    };
    EnumDropdownOption<StarterPokemon> STARTER;    
    OCR::LanguageOCROption LANGUAGE;
    PokemonBallSelectOption BALL_SELECT;
};





}
}
}
#endif
