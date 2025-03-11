/*  Orbeetle Attack Animation Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *
 *      Detects whether Orbeetle's animation is special or physical.
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_OrbeetleAttackAnimationDetector_H
#define PokemonAutomation_PokemonSwSh_OrbeetleAttackAnimationDetector_H

#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"

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
    OrbeetleAttackAnimationDetector(VideoStream& stream, ProControllerContext& context);

    Detection run(bool save_screenshot, bool log_values);


private:
    VideoStream& m_stream;
    ProControllerContext& m_context;
    OverlayBoxScope m_box;
};



}
}
}
#endif
