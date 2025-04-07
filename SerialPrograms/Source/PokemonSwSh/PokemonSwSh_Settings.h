/*  Pokemon Sword/Shield Settings
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_Settings_H
#define PokemonAutomation_PokemonSwSh_Settings_H

#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/FloatingPointOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "CommonFramework/Panels/SettingsPanel.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class GameSettings : public BatchOption{
    GameSettings();
public:
    static GameSettings& instance();

    SectionDividerOption m_egg_options;
    BooleanCheckBoxOption AUTO_DEPOSIT;
    BooleanCheckBoxOption EGG_FETCH_EXTRA_LINE;
    MillisecondsOption FETCH_EGG_MASH_DELAY0;

    SectionDividerOption m_den_options;
    BooleanCheckBoxOption DODGE_UNCATCHABLE_PROMPT_FAST;

    SectionDividerOption m_advanced_options;

    SectionDividerOption m_general_options;
    MillisecondsOption AUTO_FR_DURATION0;

    SectionDividerOption m_menu_navigation;
    MillisecondsOption OVERWORLD_TO_MENU_DELAY0;
    MillisecondsOption MENU_TO_OVERWORLD_DELAY0;
    MillisecondsOption GAME_TO_HOME_DELAY_FAST0;
    MillisecondsOption GAME_TO_HOME_DELAY_SAFE0;
    MillisecondsOption HOME_TO_GAME_DELAY0;
    MillisecondsOption OPEN_YCOMM_DELAY0;
    MillisecondsOption ENTER_PROFILE_DELAY0;

    SectionDividerOption m_start_game_timings;
    MillisecondsOption START_GAME_MASH0;
    MillisecondsOption START_GAME_WAIT0;
    MillisecondsOption ENTER_GAME_MASH0;
    MillisecondsOption ENTER_GAME_WAIT0;

    SectionDividerOption m_den_timings;
    MillisecondsOption ENTER_OFFLINE_DEN_DELAY0;
    MillisecondsOption REENTER_DEN_DELAY0;
    MillisecondsOption COLLECT_WATTS_OFFLINE_DELAY0;
    MillisecondsOption COLLECT_WATTS_ONLINE_DELAY0;
    MillisecondsOption UNCATCHABLE_PROMPT_DELAY0;
    MillisecondsOption OPEN_LOCAL_DEN_LOBBY_DELAY0;
    MillisecondsOption ENTER_SWITCH_POKEMON0;
    MillisecondsOption EXIT_SWITCH_POKEMON0;
    MillisecondsOption FULL_LOBBY_TIMER0;

    SectionDividerOption m_box_timings;
//    MillisecondsOption BOX_SCROLL_DELAY1;
    MillisecondsOption BOX_CHANGE_DELAY0;
    MillisecondsOption BOX_PICKUP_DROP_DELAY0;
    MillisecondsOption MENU_TO_POKEMON_DELAY0;
    MillisecondsOption POKEMON_TO_BOX_DELAY0;
    MillisecondsOption BOX_TO_POKEMON_DELAY0;
    MillisecondsOption POKEMON_TO_MENU_DELAY0;

    SectionDividerOption m_shiny_detection;
    FloatingPointOption SHINY_ALPHA_THRESHOLD;

    FloatingPointOption BALL_SPARKLE_ALPHA;
    FloatingPointOption STAR_SPARKLE_ALPHA;
    FloatingPointOption SQUARE_SPARKLE_ALPHA;
    FloatingPointOption LINE_SPARKLE_ALPHA;

    FloatingPointOption SHINY_DIALOG_ALPHA;
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
