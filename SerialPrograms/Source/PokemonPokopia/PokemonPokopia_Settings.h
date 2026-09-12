/*  Pokemon Pokopia
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonPokopia_Settings_H
#define PokemonAutomation_PokemonPokopia_Settings_H

#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/FloatingPointOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "CommonFramework/Panels/OptionsPanel.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonPokopia{


class GameSettings : public BatchOption{
    GameSettings();
public:
    static GameSettings& instance();

    SectionDividerOption m_general;

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
