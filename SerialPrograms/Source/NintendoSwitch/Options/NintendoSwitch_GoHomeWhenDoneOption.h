/*  Go Home When Done
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_GoHomeWhenDone_H
#define PokemonAutomation_NintendoSwitch_GoHomeWhenDone_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_Controller.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class GoHomeWhenDoneOption : public BooleanCheckBoxOption{
public:
    GoHomeWhenDoneOption(bool default_value);

    void run_end_of_program(SwitchControllerContext& context);
};


}
}
#endif
