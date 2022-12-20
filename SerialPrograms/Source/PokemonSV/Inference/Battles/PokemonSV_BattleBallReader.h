/*  Battle Ball Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_BattleBallReader_H
#define PokemonAutomation_PokemonSV_BattleBallReader_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/Language.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
//#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Inference/VisualDetector.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "Pokemon/Inference/Pokemon_PokeballNameReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;



//  Detect dialog that has the small arrow at bottom to show the next dialog.
class BattleBallReader{
public:
    BattleBallReader(ConsoleHandle& console, Language language, Color color = COLOR_RED);

    std::string read_ball(const ImageViewRGB32& screen) const;
    uint16_t read_quantity(const ImageViewRGB32& screen) const;

protected:
    const PokeballNameReader& m_name_reader;
    Language m_language;
    ConsoleHandle& m_console;
    OverlayBoxScope m_name;
    OverlayBoxScope m_sprite;
    OverlayBoxScope m_quantity;
};



}
}
}
#endif
