/*  Pokemon Legends Arceus Settings
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLA_Settings_H
#define PokemonAutomation_PokemonLA_Settings_H

#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/FloatingPointOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "CommonFramework/Panels/SettingsPanel.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class GameSettings : public BatchOption{
    GameSettings();
public:
    static GameSettings& instance();

    SectionDividerOption m_general;
    MillisecondsOption POST_WARP_DELAY0;

    SectionDividerOption m_menu_navigation;
    MillisecondsOption GAME_TO_HOME_DELAY0;
    MillisecondsOption LOAD_REGION_TIMEOUT0;

    SectionDividerOption m_start_game_timings;
    MillisecondsOption START_GAME_MASH0;
    MillisecondsOption START_GAME_WAIT1;
    MillisecondsOption ENTER_GAME_MASH0;
    MillisecondsOption ENTER_GAME_WAIT0;

    SectionDividerOption m_advanced_options;
    FloatingPointOption SHINY_SOUND_THRESHOLD;
    FloatingPointOption SHINY_SOUND_LOW_FREQUENCY;
    FloatingPointOption ALPHA_ROAR_THRESHOLD;
    FloatingPointOption ALPHA_MUSIC_THRESHOLD;
    FloatingPointOption ITEM_DROP_SOUND_THRESHOLD;
    FloatingPointOption ITEM_DROP_SOUND_LOW_FREQUENCY;
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
