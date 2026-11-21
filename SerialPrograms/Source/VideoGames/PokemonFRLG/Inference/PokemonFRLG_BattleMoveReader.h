/*  Battle Move Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Reads the four move names from the FIGHT move list, and the species of the
 *  player's Pokemon from its HP box.
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_BattleMoveReader_H
#define PokemonAutomation_PokemonFRLG_BattleMoveReader_H

#include <array>
#include <string>
#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Language.h"

namespace PokemonAutomation{
    class Logger;
    class ImageViewRGB32;
    class VideoOverlaySet;
namespace NintendoSwitch{
namespace PokemonFRLG{


class BattleMoveReader{
public:
    BattleMoveReader(Color color = COLOR_RED);

    void make_overlays(VideoOverlaySet& items) const;

    //  From the FIGHT move list. Returns the move slugs (see BattleMoveData),
    //  top-left, top-right, bottom-left, bottom-right. An empty slot ("-") or
    //  a name that can't be read is an empty string.
    //  English only: the move dictionary comes from the English game.
    std::array<std::string, 4> read_moves(Logger& logger, const ImageViewRGB32& frame) const;

    //  From the battle menu or move list. The species of the player's Pokemon,
    //  read from the name in its HP box. Empty if it can't be read.
    //  (This reads the nickname, so a nicknamed Pokemon won't be recognized.)
    std::string read_own_species(Logger& logger, Language language, const ImageViewRGB32& frame) const;

    //  The name in the player's HP box as raw text (lower case), e.g. a
    //  nickname. Empty if nothing could be read. English only.
    std::string read_own_name_text(Logger& logger, const ImageViewRGB32& frame) const;

    //  The level in the player's HP box. -1 if it can't be read.
    int read_own_level(Logger& logger, const ImageViewRGB32& frame) const;

private:
    Color m_color;
    std::array<ImageFloatBox, 4> m_move_boxes;
    ImageFloatBox m_own_name_box;
    ImageFloatBox m_own_level_box;
};



}
}
}
#endif
