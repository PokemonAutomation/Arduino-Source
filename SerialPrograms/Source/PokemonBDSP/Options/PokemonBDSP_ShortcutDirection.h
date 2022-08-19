/*  Shortcut Direction
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "ClientSource/Connection/BotBase.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"

#ifndef PokemonAutomation_PokemonBDSP_ShortcutDirection_H
#define PokemonAutomation_PokemonBDSP_ShortcutDirection_H

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{



enum class ShortcutDirection{
    NONE,
    UP,
    RIGHT,
    DOWN,
    LEFT,
};
const EnumDatabase<ShortcutDirection>& ShortcutDirection_Nullable();
const EnumDatabase<ShortcutDirection>& ShortcutDirection_Required();



class ShortcutDirectionOption : public EnumDropdownOption<ShortcutDirection>{
public:
    ShortcutDirectionOption(std::string label);

    void run(BotBaseContext& context, uint16_t delay);

};



}
}
}
#endif
