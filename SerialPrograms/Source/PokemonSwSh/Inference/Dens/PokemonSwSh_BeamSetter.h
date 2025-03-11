/*  Beam Setter
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *
 *      Drop a wishing piece and determine if it is red or purple.
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_BeamSetter_H
#define PokemonAutomation_PokemonSwSh_BeamSetter_H

#include <vector>
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"

namespace PokemonAutomation{
    class ProgramEnvironment;
namespace NintendoSwitch{
namespace PokemonSwSh{


class BeamSetter{
public:
    enum Detection{
        NO_DETECTION,
        RED_DETECTED,
        RED_ASSUMED,
        PURPLE,
    };

public:
    BeamSetter(
        ProgramEnvironment& env,
        VideoStream& stream, ProControllerContext& context
    );

    Detection run(
        bool save_screenshot,
        Milliseconds timeout,
        double min_brightness,
        double min_euclidean,
        double min_delta_ratio,
        double min_sigma_ratio
    );


private:
    VideoStream& m_stream;
    ProControllerContext& m_context;
    OverlayBoxScope m_text_box0;
    OverlayBoxScope m_text_box1;
    OverlayBoxScope m_box;
    std::vector<ImageFloatBox> m_boxes;
};




}
}
}
#endif
