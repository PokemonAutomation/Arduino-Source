/*  Video Fast Code Entry
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_VideoFastCodeEntry_H
#define PokemonAutomation_PokemonSV_VideoFastCodeEntry_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "CommonFramework/Options/ScreenWatchOption.h"
#include "NintendoSwitch/NintendoSwitch_MultiSwitchProgram.h"
#include "NintendoSwitch/Programs/NintendoSwitch_FastCodeEntry.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



class VideoFastCodeEntry_Descriptor : public MultiSwitchProgramDescriptor{
public:
    VideoFastCodeEntry_Descriptor();
};




class VideoFastCodeEntry : public MultiSwitchProgramInstance{
public:
    VideoFastCodeEntry();
    virtual void program(MultiSwitchProgramEnvironment& env, CancellableScope& scope) override;

private:
    ScreenWatchOption SCREEN_WATCHER;
    BooleanCheckBoxOption SKIP_CONNECT_TO_CONTROLLER;
    FastCodeEntrySettingsOption SETTINGS;
};




}
}
}
#endif
