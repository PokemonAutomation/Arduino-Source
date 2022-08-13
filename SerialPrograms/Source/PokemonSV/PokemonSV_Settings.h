/*  Pokemon SV Settings
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_Settings_H
#define PokemonAutomation_PokemonSV_Settings_H

#include "CommonFramework/Options/BooleanCheckBoxOption.h"
#include "CommonFramework/Options/FloatingPointOption.h"
#include "CommonFramework/Options/StaticTextOption.h"
#include "CommonFramework/Panels/SettingsPanel.h"
#include "NintendoSwitch/Options/TimeExpressionOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


class GameSettings : public BatchOption{
    GameSettings();
public:
    static GameSettings& instance();

    SectionDividerOption m_start_game_timings;
    TimeExpressionOption<uint16_t> START_GAME_MASH;
    TimeExpressionOption<uint16_t> START_GAME_WAIT;
    TimeExpressionOption<uint16_t> ENTER_GAME_MASH;
    TimeExpressionOption<uint16_t> ENTER_GAME_WAIT;
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
