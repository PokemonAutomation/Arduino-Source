/*  Go Home When Done
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_GoHomeWhenDone_H
#define PokemonAutomation_GoHomeWhenDone_H

#include "CommonFramework/Options/BooleanCheckBoxOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class GoHomeWhenDoneOption : public BooleanCheckBoxOption{
public:
    GoHomeWhenDoneOption(bool default_value)
        : BooleanCheckBoxOption(
            "<b>Go Home when Done:</b><br>"
            "After finding a shiny, go to the Switch Home menu to idle. (turn this off for unattended streaming)",
            default_value
        )
    {}
};


}
}
#endif
