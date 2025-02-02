/*  Pokemon SV Settings
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_Settings_H
#define PokemonAutomation_PokemonSV_Settings_H

#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/FloatingPointOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "Common/Cpp/Options/TimeExpressionOption.h"
#include "CommonFramework/Panels/SettingsPanel.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


class GameSettings : public BatchOption{
    GameSettings();
public:
    static GameSettings& instance();

    SectionDividerOption m_menu_navigation;
    TimeExpressionOption<uint16_t> GAME_TO_HOME_DELAY;

    SectionDividerOption m_start_game_timings;
    MillisecondsOption START_GAME_MASH0;
    TimeExpressionOption<uint16_t> START_GAME_WAIT;
    TimeExpressionOption<uint16_t> ENTER_GAME_MASH;
    TimeExpressionOption<uint16_t> ENTER_GAME_WAIT;

    SectionDividerOption m_advanced_options;
    TimeExpressionOption<uint16_t> RAID_SPAWN_DELAY;
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
