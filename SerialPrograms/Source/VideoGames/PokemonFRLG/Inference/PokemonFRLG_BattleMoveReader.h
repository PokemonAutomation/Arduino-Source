/*  Battle Move Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  In a single battle: reads the four move names from the FIGHT move list,
 *  and the name and level of the player's Pokemon from its HP box.
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_BattleMoveReader_H
#define PokemonAutomation_PokemonFRLG_BattleMoveReader_H

#include <array>
#include <string>
#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Language.h"
#include "CommonTools/OCR/OCR_SmallDictionaryMatcher.h"

namespace PokemonAutomation{
    class Logger;
    class ImageViewRGB32;
    class VideoOverlaySet;
namespace NintendoSwitch{
namespace PokemonFRLG{


//  The Gen 3 move names in each language, from
//  PokemonFRLG/BattleMovesOCR.json. Tokens are the move slugs of
//  PokemonFRLG/BattleMoves.json.
class BattleMoveOCR : public OCR::SmallDictionaryMatcher{
public:
    static constexpr double MAX_LOG10P = -1.40;
    static constexpr double MAX_LOG10P_SPREAD = 0.50;

    static const BattleMoveOCR& instance();

private:
    BattleMoveOCR();
};


class BattleMoveReader{
public:
    BattleMoveReader(Language language, Color color = COLOR_RED);

    void make_overlays(VideoOverlaySet& items) const;

    //  From the FIGHT move list: the move slugs, top-left, top-right,
    //  bottom-left, bottom-right. An empty slot ("-") or a name that can't be
    //  read is an empty string.
    std::array<std::string, 4> read_moves(Logger& logger, const ImageViewRGB32& frame) const;

    //  From the battle menu or move list: the species of the player's
    //  Pokemon, read from the name in its HP box. Empty if it can't be read,
    //  e.g. if the Pokemon has a nickname.
    std::string read_own_species(Logger& logger, const ImageViewRGB32& frame) const;

    //  The name in the player's HP box (e.g. a nickname) as raw text. Empty
    //  if nothing could be read.
    std::string read_own_name_text(Logger& logger, const ImageViewRGB32& frame) const;

    //  The level in the player's HP box. -1 if it can't be read.
    int read_own_level(Logger& logger, const ImageViewRGB32& frame) const;

private:
    Language m_language;
    Color m_color;
    std::array<ImageFloatBox, 4> m_move_boxes;
    ImageFloatBox m_own_name_box;
    ImageFloatBox m_own_level_box;
};



}
}
}
#endif
