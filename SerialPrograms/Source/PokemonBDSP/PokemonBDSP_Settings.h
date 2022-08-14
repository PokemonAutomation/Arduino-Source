/*  Pokemon BDSP Settings
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_Settings_H
#define PokemonAutomation_PokemonBDSP_Settings_H

#include "Common/Qt/Options/StaticTextOption.h"
#include "Common/Qt/Options/BooleanCheckBoxOption.h"
#include "Common/Qt/Options/FloatingPointOption.h"
#include "CommonFramework/Panels/SettingsPanel.h"
#include "NintendoSwitch/Options/TimeExpressionOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class GameSettings : public BatchOption{
    GameSettings();
public:
    static GameSettings& instance();

    SectionDividerOption m_menu_navigation;
    TimeExpressionOption<uint16_t> OVERWORLD_TO_MENU_DELAY;
    TimeExpressionOption<uint16_t> MENU_TO_OVERWORLD_DELAY;
    TimeExpressionOption<uint16_t> GAME_TO_HOME_DELAY;

    SectionDividerOption m_start_game_timings;
    TimeExpressionOption<uint16_t> START_GAME_MASH;
    TimeExpressionOption<uint16_t> START_GAME_WAIT;
    TimeExpressionOption<uint16_t> ENTER_GAME_MASH;
    TimeExpressionOption<uint16_t> ENTER_GAME_WAIT;

    SectionDividerOption m_box_timings;
    TimeExpressionOption<uint16_t> BOX_SCROLL_DELAY_0;      //  Renamed to force defaults on version upgrade.
    TimeExpressionOption<uint16_t> BOX_CHANGE_DELAY_0;      //  Renamed to force defaults on version upgrade.
    TimeExpressionOption<uint16_t> BOX_PICKUP_DROP_DELAY;
    TimeExpressionOption<uint16_t> MENU_TO_POKEMON_DELAY;
    TimeExpressionOption<uint16_t> POKEMON_TO_BOX_DELAY0;
    TimeExpressionOption<uint16_t> BOX_TO_POKEMON_DELAY;
    TimeExpressionOption<uint16_t> POKEMON_TO_MENU_DELAY;

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
