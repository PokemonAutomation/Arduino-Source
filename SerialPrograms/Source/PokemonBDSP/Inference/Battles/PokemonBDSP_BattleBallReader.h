/*  In-Battle Ball Inventory Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_BattleBallInventoryReader_H
#define PokemonAutomation_PokemonBDSP_BattleBallInventoryReader_H

#include <string>
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "CommonFramework/ImageMatch/CroppedImageDictionaryMatcher.h"
#include "Pokemon/Inference/Pokemon_PokeballNameReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{
using namespace Pokemon;


class BattleBallReader{
    static const double MAX_ALPHA;
    static const double ALPHA_SPREAD;

public:
    BattleBallReader(ConsoleHandle& console, Language language);

public:
    std::string read_ball(const ImageViewRGB32& screen) const;
    uint16_t read_quantity(const ImageViewRGB32& screen) const;

private:
    const ImageMatch::CroppedImageDictionaryMatcher& m_matcher;
    const PokeballNameReader& m_name_reader;
    Language m_language;
    ConsoleHandle& m_console;
    OverlayBoxScope m_box_sprite;
    OverlayBoxScope m_box_name;
    OverlayBoxScope m_box_quantity;
};




}
}
}
#endif
