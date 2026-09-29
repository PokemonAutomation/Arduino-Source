/*  In-Battle Ball Inventory Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_BattleBallInventoryReader_H
#define PokemonAutomation_PokemonSwSh_BattleBallInventoryReader_H

#include <string>
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "CommonTools/ImageMatch/ExactImageDictionaryMatcher.h"
#include "Pokemon/Inference/Pokemon_PokeballNameReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
using namespace Pokemon;


class BattleBallReader{
    static const double MAX_ALPHA;
    static const double ALPHA_SPREAD;

public:
    BattleBallReader(
        VideoStream& stream,
        Language language
    );

public:
    std::string read_ball(const ImageViewRGB32& screen) const;
    uint16_t read_quantity(const ImageViewRGB32& screen) const;

private:
    const ImageMatch::ExactImageDictionaryMatcher& m_matcher;
    const PokeballNameReader& m_name_reader;
    Language m_language;
    VideoStream& m_stream;
    OverlayBoxScope m_box_sprite;
    OverlayBoxScope m_box_name;
    OverlayBoxScope m_box_quantity;
};




}
}
}
#endif
