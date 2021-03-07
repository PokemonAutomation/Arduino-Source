/*  Pokemon Settings Panel
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSettingsPanel_H
#define PokemonAutomation_PokemonSettingsPanel_H

#include "CommonFramework/Panels/SettingsPanel.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class PokemonSettings : public SettingsPanel{
public:
    PokemonSettings();
    PokemonSettings(const QJsonValue& json);
};


}
}
}
#endif
