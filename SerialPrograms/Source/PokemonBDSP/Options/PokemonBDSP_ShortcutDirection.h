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
    ShortcutDirection(std::string label, bool required = true);

    void run(BotBaseContext& context, uint16_t delay);

private:
    bool m_required = true;
};



}
}
}
#endif
