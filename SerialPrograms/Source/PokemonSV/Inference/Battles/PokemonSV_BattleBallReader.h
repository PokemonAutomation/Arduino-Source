/*  Battle Ball Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_BattleBallReader_H
#define PokemonAutomation_PokemonSV_BattleBallReader_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/Language.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "CommonTools/VisualDetector.h"
#include "Pokemon/Inference/Pokemon_PokeballNameReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;



//  Detect dialog that has the small arrow at bottom to show the next dialog.
class BattleBallReader{
public:
    BattleBallReader(VideoStream& stream, Language language, Color color = COLOR_RED);

    std::string read_ball(const ImageViewRGB32& screen) const;
    uint16_t read_quantity(const ImageViewRGB32& screen) const;

protected:
    const PokeballNameReader& m_name_reader;
    Language m_language;
    Logger& m_logger;
    OverlayBoxScope m_name;
    OverlayBoxScope m_sprite;
    OverlayBoxScope m_quantity;
};



}
}
}
#endif
