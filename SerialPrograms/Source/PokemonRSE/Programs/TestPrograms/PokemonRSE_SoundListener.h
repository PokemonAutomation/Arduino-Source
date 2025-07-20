/*  Sound Listener
 *
 *  From: https://github.com/PokemonAutomation/
 *  
 *  Debug program to test all kinds of sound detectors.
 */

#ifndef PokemonAutomation_PokemonRSE_SoundListener_H
#define PokemonAutomation_PokemonRSE_SoundListener_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
//#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"

namespace PokemonAutomation{ 
namespace NintendoSwitch{
namespace PokemonRSE{


class SoundListener_Descriptor : public SingleSwitchProgramDescriptor{
public:
    SoundListener_Descriptor();
};


class SoundListener : public SingleSwitchProgramInstance{
public:
    SoundListener();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

    virtual void start_program_border_check(
        VideoStream& stream,
        FeedbackType feedback_type
    ) override{}

private:
    enum class SoundType{
        SHINY,
    };

    EnumDropdownOption<SoundType> SOUND_TYPE;
    BooleanCheckBoxOption STOP_ON_DETECTED_SOUND;
};




}
}
}
#endif
