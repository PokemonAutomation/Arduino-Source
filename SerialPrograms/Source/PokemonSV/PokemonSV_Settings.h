/*  Pokemon SV Settings
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_Settings_H
#define PokemonAutomation_PokemonSV_Settings_H

#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/FloatingPointOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "CommonFramework/Panels/SettingsPanel.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


class GameSettings : public BatchOption{
    GameSettings();
public:
    static GameSettings& instance();

    SectionDividerOption m_menu_navigation;
    MillisecondsOption GAME_TO_HOME_DELAY1;

    SectionDividerOption m_start_game_timings;
    MillisecondsOption START_GAME_MASH0;
    MillisecondsOption START_GAME_WAIT0;
    MillisecondsOption ENTER_GAME_MASH0;
    MillisecondsOption ENTER_GAME_WAIT0;

    SectionDividerOption m_advanced_options;
    MillisecondsOption RAID_SPAWN_DELAY0;
    FloatingPointOption SHINY_SOUND_THRESHOLD2;
    FloatingPointOption SHINY_SOUND_LOW_FREQUENCY;
    FloatingPointOption LETS_GO_KILL_SOUND_THRESHOLD;
    FloatingPointOption LETS_GO_KILL_SOUND_LOW_FREQUENCY;
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
