/*  Orbeetle Attack Animation Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *
 *      Detects whether Orbeetle's animation is special or physical.
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_OrbeetleAttackAnimationDetector_H
#define PokemonAutomation_PokemonSwSh_OrbeetleAttackAnimationDetector_H

#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_Controller.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"

namespace PokemonAutomation{
class ProgramEnvironment;
namespace NintendoSwitch{
namespace PokemonSwSh{


class OrbeetleAttackAnimationDetector{
public:
    enum Detection{
        NO_DETECTION,
        SPECIAL,
        PHYSICAL,
    };

public:
    OrbeetleAttackAnimationDetector(ConsoleHandle& console, SwitchControllerContext& context);

    Detection run(bool save_screenshot, bool log_values);


private:
    ConsoleHandle& m_console;
    SwitchControllerContext& m_context;
    OverlayBoxScope m_box;
};



}
}
}
#endif
