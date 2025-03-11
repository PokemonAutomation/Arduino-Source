/*  Learn Move
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_LearnMove_H
#define PokemonAutomation_PokemonBDSP_LearnMove_H

#include "Common/Cpp/Options/EnumDropdownOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


enum class OnLearnMove{
    DONT_LEARN,
    STOP_PROGRAM,
};

class OnLearnMoveOption : public EnumDropdownOption<OnLearnMove>{
public:
    OnLearnMoveOption()
        : EnumDropdownOption<OnLearnMove>(
            "<b>On Learn Move:</b>",
            {
                {OnLearnMove::DONT_LEARN, "skip", "Don't learn moves."},
                {OnLearnMove::STOP_PROGRAM, "stop-program", "Stop Program"},
            },
            LockMode::LOCK_WHILE_RUNNING,
            OnLearnMove::DONT_LEARN
        )
    {}
};



}
}
}
#endif
