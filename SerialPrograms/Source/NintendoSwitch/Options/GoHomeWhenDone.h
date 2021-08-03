/*  Go Home When Done
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_GoHomeWhenDone_H
#define PokemonAutomation_GoHomeWhenDone_H

#include "CommonFramework/Options/BooleanCheckBox.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class GoHomeWhenDone : public BooleanCheckBox{
public:
    GoHomeWhenDone(bool default_value)
        : BooleanCheckBox(
            "<b>Go Home when Done:</b><br>"
            "After finding a shiny, go to the Switch Home menu to idle. (turn this off for unattended streaming)",
            default_value
        )
    {}
};


}
}
#endif
