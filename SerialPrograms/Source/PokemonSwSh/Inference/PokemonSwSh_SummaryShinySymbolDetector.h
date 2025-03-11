/*  Summary Shiny Symbol
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *
 *      Check shiny symbol in the party pokemon info screen.
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_SummaryShinySymbolDetector_H
#define PokemonAutomation_PokemonSwSh_SummaryShinySymbolDetector_H

#include <chrono>
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"

namespace PokemonAutomation{
    class CancellableScope;
    class VideoFeed;
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
    SummaryShinySymbolDetector(Logger& logger, VideoOverlay& overlay);

    Detection detect(const ImageViewRGB32& screen);
    Detection wait_for_detection(
        CancellableScope& scope, VideoFeed& feed,
        std::chrono::seconds timeout = std::chrono::seconds(10)
    );

private:
    Logger& m_logger;
    OverlayBoxScope m_state0_box;
    OverlayBoxScope m_state1_box;
    OverlayBoxScope m_symbol_box;
};



}
}
}

#endif

