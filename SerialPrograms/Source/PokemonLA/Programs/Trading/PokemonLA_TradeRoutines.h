/*  Trade Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_TradeRoutines_H
#define PokemonAutomation_PokemonLA_TradeRoutines_H

#include "CommonFramework/Language.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "CommonTools/MultiConsoleErrors.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_Controller.h"

namespace PokemonAutomation{
    class ProgramEnvironment;
namespace NintendoSwitch{
namespace PokemonLA{


struct TradeStats : public StatsTracker{
    TradeStats();
    std::atomic<uint64_t>& m_trades;
    std::atomic<uint64_t>& m_errors;
};


void trade_current_pokemon(
    VideoStream& stream, SwitchControllerContext& context,
    MultiConsoleErrorState& tracker,
    TradeStats& stats
);



class TradeNameReader{
public:
    TradeNameReader(Logger& logger, VideoOverlay& overlay, Language language);

    std::string read(const ImageViewRGB32& screen) const;

private:
    Logger& m_logger;
    Language m_language;
    OverlayBoxScope m_box;
    std::set<std::string> m_slugs;
};




}
}
}
#endif
