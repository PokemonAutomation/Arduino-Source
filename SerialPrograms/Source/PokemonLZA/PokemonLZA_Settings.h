/*  Pokemon Legends ZA
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_Settings_H
#define PokemonAutomation_PokemonLZA_Settings_H

#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "CommonFramework/Panels/OptionsPanel.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{


class GameSettings : public BatchOption{
    GameSettings();
public:
    static GameSettings& instance();

    SectionDividerOption m_general;

    SectionDividerOption m_start_game_timings;
    MillisecondsOption START_GAME_WAIT;
    MillisecondsOption ENTER_GAME_WAIT0;

    SectionDividerOption m_advanced_options;


};




class GameSettings_Descriptor : public OptionsPanelDescriptor{
public:
    GameSettings_Descriptor();
};


class GameSettingsPanel : public OptionsPanelInstance{
public:
    GameSettingsPanel();
private:
    GameSettings& settings;
};




}
}
}
#endif
