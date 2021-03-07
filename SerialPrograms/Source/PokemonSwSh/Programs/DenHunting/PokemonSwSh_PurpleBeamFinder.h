/*  Purple Beam Finder
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_PurpleBeamFinder_H
#define PokemonAutomation_PokemonSwSh_PurpleBeamFinder_H

#include "CommonFramework/Options/BooleanCheckBox.h"
#include "CommonFramework/Options/FloatingPoint.h"
#include "NintendoSwitch/Options/TimeExpression.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

class PurpleBeamFinder : public SingleSwitchProgram{
public:
    PurpleBeamFinder();

    virtual void program(SingleSwitchProgramEnvironment& env) const override;

private:
    BooleanCheckBox EXTRA_LINE;
    FloatingPoint DETECTION_THRESHOLD;
    TimeExpression<uint16_t> TIMEOUT_DELAY;
};


}
}
}
#endif

