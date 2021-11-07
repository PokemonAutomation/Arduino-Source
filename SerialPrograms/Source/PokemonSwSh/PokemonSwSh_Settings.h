/*  Pokemon Sword/Shield Settings
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_Settings_H
#define PokemonAutomation_PokemonSwSh_Settings_H

#include "CommonFramework/Options/StaticTextOption.h"
#include "CommonFramework/Options/BooleanCheckBoxOption.h"
#include "CommonFramework/Panels/SettingsPanel.h"
#include "NintendoSwitch/Options/TimeExpressionOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

enum class Catchability{
    ALWAYS_CATCHABLE,
    MAYBE_UNCATCHABLE,
    ALWAYS_UNCATCHABLE,
};


class GameSettings : public BatchOption{
    GameSettings();
public:
    static GameSettings& instance();

    SectionDividerOption m_egg_options;
    BooleanCheckBoxOption AUTO_DEPOSIT;
    BooleanCheckBoxOption EGG_FETCH_EXTRA_LINE;
    TimeExpressionOption<uint16_t> FETCH_EGG_MASH_DELAY;

    SectionDividerOption m_den_options;
    BooleanCheckBoxOption DODGE_UNCATCHABLE_PROMPT_FAST;

    SectionDividerOption m_advanced_options;

    SectionDividerOption m_general_options;
    TimeExpressionOption<uint16_t> AUTO_FR_DURATION;

    SectionDividerOption m_menu_navigation;
    TimeExpressionOption<uint16_t> OVERWORLD_TO_MENU_DELAY;
    TimeExpressionOption<uint16_t> MENU_TO_OVERWORLD_DELAY;
    TimeExpressionOption<uint16_t> GAME_TO_HOME_DELAY_FAST;
    TimeExpressionOption<uint16_t> GAME_TO_HOME_DELAY_SAFE;
    TimeExpressionOption<uint16_t> HOME_TO_GAME_DELAY;
    TimeExpressionOption<uint16_t> OPEN_YCOMM_DELAY;
    TimeExpressionOption<uint16_t> ENTER_PROFILE_DELAY;

    SectionDividerOption m_start_game_timings;
    TimeExpressionOption<uint16_t> START_GAME_MASH;
    TimeExpressionOption<uint16_t> START_GAME_WAIT;
    TimeExpressionOption<uint16_t> ENTER_GAME_MASH;
    TimeExpressionOption<uint16_t> ENTER_GAME_WAIT;

    SectionDividerOption m_den_timings;
    TimeExpressionOption<uint16_t> ENTER_OFFLINE_DEN_DELAY;
    TimeExpressionOption<uint16_t> REENTER_DEN_DELAY;
    TimeExpressionOption<uint16_t> COLLECT_WATTS_OFFLINE_DELAY;
    TimeExpressionOption<uint16_t> COLLECT_WATTS_ONLINE_DELAY;
    TimeExpressionOption<uint16_t> UNCATCHABLE_PROMPT_DELAY;
    TimeExpressionOption<uint16_t> OPEN_LOCAL_DEN_LOBBY_DELAY;
    TimeExpressionOption<uint16_t> ENTER_SWITCH_POKEMON;
    TimeExpressionOption<uint16_t> EXIT_SWITCH_POKEMON;
    TimeExpressionOption<uint16_t> FULL_LOBBY_TIMER;

    SectionDividerOption m_box_timings;
    TimeExpressionOption<uint16_t> BOX_SCROLL_DELAY;
    TimeExpressionOption<uint16_t> BOX_CHANGE_DELAY;
    TimeExpressionOption<uint16_t> BOX_PICKUP_DROP_DELAY;
    TimeExpressionOption<uint16_t> MENU_TO_POKEMON_DELAY;
    TimeExpressionOption<uint16_t> POKEMON_TO_BOX_DELAY;
    TimeExpressionOption<uint16_t> BOX_TO_POKEMON_DELAY;
    TimeExpressionOption<uint16_t> POKEMON_TO_MENU_DELAY;
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
