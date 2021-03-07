/*  Framework Settings Panel
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_FrameworkSettingsPanel_H
#define PokemonAutomation_FrameworkSettingsPanel_H

#include "CommonFramework/Panels/SettingsPanel.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class FrameworkSettings : public SettingsPanel{
public:
    FrameworkSettings();
    FrameworkSettings(const QJsonValue& json);
};


}
}
#endif
