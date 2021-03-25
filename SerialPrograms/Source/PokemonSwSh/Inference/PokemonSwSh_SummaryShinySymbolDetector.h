/*  Summary Shiny Symbol
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *
 *      Determine if a raid is full and ready to start early.
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_SummaryShinySymbolDetector_H
#define PokemonAutomation_PokemonSwSh_SummaryShinySymbolDetector_H

#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/Tools/Logger.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class SummaryShinySymbolDetector{
public:
    enum Detection{
        NO_DETECTION,
        NOT_SHINY,
        SHINY,
    };

public:
    SummaryShinySymbolDetector(VideoFeed& feed, Logger& logger);

    Detection detect_now();
    Detection wait_for_detection(
        ProgramEnvironment& env,
        std::chrono::seconds timeout = std::chrono::seconds(10)
    );

private:
    VideoFeed& m_feed;
    Logger& m_logger;
    InferenceBoxScope m_state0_box;
    InferenceBoxScope m_state1_box;
    InferenceBoxScope m_symbol_box;
};



}
}
}

#endif

