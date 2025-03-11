/*  Pokemon BDSP Settings
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_Settings_H
#define PokemonAutomation_PokemonBDSP_Settings_H

#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/FloatingPointOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "CommonFramework/Panels/SettingsPanel.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class GameSettings : public BatchOption{
    GameSettings();
public:
    static GameSettings& instance();

    SectionDividerOption m_menu_navigation;
    MillisecondsOption OVERWORLD_TO_MENU_DELAY0;
    MillisecondsOption MENU_TO_OVERWORLD_DELAY0;
    MillisecondsOption GAME_TO_HOME_DELAY0;

    SectionDividerOption m_start_game_timings;
    MillisecondsOption START_GAME_MASH0;
    MillisecondsOption START_GAME_WAIT0;
    MillisecondsOption ENTER_GAME_MASH0;
    MillisecondsOption ENTER_GAME_WAIT0;

    SectionDividerOption m_box_timings;
    MillisecondsOption BOX_SCROLL_DELAY0;
    MillisecondsOption BOX_CHANGE_DELAY0;
    MillisecondsOption BOX_PICKUP_DROP_DELAY0;
    MillisecondsOption MENU_TO_POKEMON_DELAY0;
    MillisecondsOption POKEMON_TO_BOX_DELAY1;
    MillisecondsOption BOX_TO_POKEMON_DELAY0;
    MillisecondsOption POKEMON_TO_MENU_DELAY0;

    SectionDividerOption m_advanced_options;
    FloatingPointOption SHINY_ALPHA_OVERALL_THRESHOLD;
    FloatingPointOption SHINY_ALPHA_SIDE_THRESHOLD;

    FloatingPointOption BALL_SPARKLE_ALPHA;
    FloatingPointOption STAR_SPARKLE_ALPHA;

    FloatingPointOption SHINY_DIALOG_ALPHA;

    FloatingPointOption SHINY_SOUND_THRESHOLD;
    FloatingPointOption SHINY_SOUND_LOW_FREQUENCY;
    FloatingPointOption SHINY_SOUND_ALPHA;
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
