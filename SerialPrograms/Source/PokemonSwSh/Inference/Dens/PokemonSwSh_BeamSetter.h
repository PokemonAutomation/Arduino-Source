/*  Beam Setter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
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
#include "NintendoSwitch/Controllers/NintendoSwitch_Controller.h"

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
        VideoStream& stream, SwitchControllerContext& context
    );

    Detection run(
        bool save_screenshot,
        uint16_t timeout_ticks,
        double min_brightness,
        double min_euclidean,
        double min_delta_ratio,
        double min_sigma_ratio
    );


private:
    VideoStream& m_stream;
    SwitchControllerContext& m_context;
    OverlayBoxScope m_text_box0;
    OverlayBoxScope m_text_box1;
    OverlayBoxScope m_box;
    std::vector<ImageFloatBox> m_boxes;
};




}
}
}
#endif
