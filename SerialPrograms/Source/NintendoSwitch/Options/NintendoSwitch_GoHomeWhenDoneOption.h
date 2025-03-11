/*  Go Home When Done
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_GoHomeWhenDone_H
#define PokemonAutomation_NintendoSwitch_GoHomeWhenDone_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class GoHomeWhenDoneOption : public BooleanCheckBoxOption{
public:
    GoHomeWhenDoneOption(bool default_value);

    void run_end_of_program(ProControllerContext& context);
};


}
}
#endif
