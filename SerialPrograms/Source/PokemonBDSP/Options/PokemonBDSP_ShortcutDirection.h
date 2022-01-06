/*  Shortcut Direction
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "ClientSource/Connection/BotBase.h"
#include "CommonFramework/Options/EnumDropdownOption.h"

#ifndef PokemonAutomation_PokemonBDSP_ShortcutDirection_H
#define PokemonAutomation_PokemonBDSP_ShortcutDirection_H

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class ShortcutDirection : public EnumDropdownOption{
public:
    ShortcutDirection(QString label);

    void run(const BotBaseContext& context, uint16_t delay);
};



}
}
}
#endif
