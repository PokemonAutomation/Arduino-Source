/*  Go Home When Done
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_GoHomeWhenDone_H
#define PokemonAutomation_GoHomeWhenDone_H

#include "ClientSource/Connection/BotBase.h"
#include "CommonFramework/Options/BooleanCheckBoxOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class GoHomeWhenDoneOption : public BooleanCheckBoxOption{
public:
    GoHomeWhenDoneOption(bool default_value);

    void run_end_of_program(BotBaseContext& context);
};


}
}
#endif
