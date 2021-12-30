/*  In-Battle Ball Inventory Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_BattleBallInventoryReader_H
#define PokemonAutomation_PokemonSwSh_BattleBallInventoryReader_H

#include <string>
#include "CommonFramework/Options/StringSelectOption.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/ImageMatch/ExactImageDictionaryMatcher.h"
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
        ConsoleHandle& console,
        Language language
    );

public:
    std::string read_ball(const QImage& screen) const;
    uint16_t read_quantity(const QImage& screen) const;

private:
    const ImageMatch::ExactImageDictionaryMatcher& m_matcher;
    const PokeballNameReader& m_name_reader;
    Language m_language;
    ConsoleHandle& m_console;
    InferenceBoxScope m_box_sprite;
    InferenceBoxScope m_box_name;
    InferenceBoxScope m_box_quantity;
};




}
}
}
#endif
