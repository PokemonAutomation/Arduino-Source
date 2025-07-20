/*  Pokemon Let's Go Settings
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLGPE_Settings_H
#define PokemonAutomation_PokemonLGPE_Settings_H

#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/FloatingPointOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "CommonFramework/Panels/SettingsPanel.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLGPE{


class GameSettings : public BatchOption{
    GameSettings();
public:
    static GameSettings& instance();

    SectionDividerOption m_general;

    SectionDividerOption m_menu_navigation;
    MillisecondsOption GAME_TO_HOME_DELAY0;

    SectionDividerOption m_start_game_timings;
    MillisecondsOption START_GAME_MASH0;
    MillisecondsOption START_GAME_WAIT1;
    MillisecondsOption ENTER_GAME_MASH0;
    MillisecondsOption ENTER_GAME_WAIT0;

    SectionDividerOption m_shiny_audio_settings;
    FloatingPointOption SHINY_SOUND_THRESHOLD;
    FloatingPointOption SHINY_SOUND_LOW_FREQUENCY;
};




class GameSettings_Descriptor : public PanelDescriptor{
public:
    GameSettings_Descriptor();
};


class GameSettingsPanel : public SettingsPanelInstance{
public:
    GameSettingsPanel(const GameSettings_Descriptor& descriptor);
private:
    GameSettings& settings;
};


}
}
}
#endif
