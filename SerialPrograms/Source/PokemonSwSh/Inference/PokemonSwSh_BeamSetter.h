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
#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/Tools/Logger.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"

namespace PokemonAutomation{
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
    BeamSetter(VideoFeed& feed, Logger& logger);

    Detection run(
        ProgramEnvironment& env, BotBase& botbase,
        double detection_threshold, uint16_t timeout_ticks
    );


private:
    VideoFeed& m_feed;
    Logger& m_logger;
    InferenceBoxScope m_text_box;
    InferenceBoxScope m_box;
    std::vector<InferenceBox> m_boxes;
};




}
}
}
#endif
