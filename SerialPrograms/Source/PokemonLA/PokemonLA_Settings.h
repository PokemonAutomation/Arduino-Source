/*  Pokemon Legends Arceus Settings
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_Settings_H
#define PokemonAutomation_PokemonLA_Settings_H

#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/FloatingPointOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "Common/Cpp/Options/TimeExpressionOption.h"
#include "CommonFramework/Panels/SettingsPanel.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class GameSettings : public BatchOption{
    GameSettings();
public:
    static GameSettings& instance();

    SectionDividerOption m_general;
    FloatingPointOption POST_WARP_DELAY;

    SectionDividerOption m_menu_navigation;
//    TimeExpressionOption<uint16_t> OVERWORLD_TO_MENU_DELAY;
//    TimeExpressionOption<uint16_t> MENU_TO_OVERWORLD_DELAY;
    TimeExpressionOption<uint16_t> GAME_TO_HOME_DELAY;
    TimeExpressionOption<uint16_t> LOAD_REGION_TIMEOUT;

    SectionDividerOption m_start_game_timings;
    MillisecondsOption START_GAME_MASH0;
    MillisecondsOption START_GAME_WAIT1;
    TimeExpressionOption<uint16_t> ENTER_GAME_MASH;
    TimeExpressionOption<uint16_t> ENTER_GAME_WAIT;

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
