/*  Tera Catch On Win Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_TeraCatchOnWinOption_H
#define PokemonAutomation_PokemonSV_TeraCatchOnWinOption_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/GroupOption.h"
#include "PokemonSwSh/Options/PokemonSwSh_BallSelectOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



class TeraFarmerCatchOnWin : public GroupOption{
public:
    TeraFarmerCatchOnWin(bool enabled);

public:
    PokemonSwSh::PokemonBallSelectOption BALL_SELECT;
    BooleanCheckBoxOption FIX_TIME_ON_CATCH;
};




}
}
}
#endif
