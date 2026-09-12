/*  Pokemon Home Settings
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonHome_Settings_H
#define PokemonAutomation_PokemonHome_Settings_H

#include "CommonFramework/Panels/OptionsPanel.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{


class GameSettings : public BatchOption{
    GameSettings();
public:
    static GameSettings& instance();

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
