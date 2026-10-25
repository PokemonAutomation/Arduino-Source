/*  Box Cell Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_BoxCellReader_H
#define PokemonAutomation_PokemonFRLG_BoxCellReader_H

#include <string>
#include "Common/Cpp/Color.h"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Language.h"

namespace PokemonAutomation{

class ImageViewRGB32;
class VideoOverlaySet;

namespace NintendoSwitch{
namespace PokemonFRLG{

// Reads the species and level displayed for the currently selected box cell.
class BoxCellReader{
public:
    BoxCellReader(Color color = COLOR_RED);

    void make_overlays(VideoOverlaySet& items) const;

    // Returns a species slug from National Dex 1-386, or "" if empty/unreadable.
    std::string read_name(Logger& logger, Language language, const ImageViewRGB32& screen) const;

    // Reads the number alongside Lv. Returns 1-100, or -1 if empty/unreadable.
    int read_level(Logger& logger, const ImageViewRGB32& screen) const;

private:
    Color m_color;
    ImageFloatBox m_name_box;
    ImageFloatBox m_level_box;
};

void add_tests_BoxCellReader(UnitTestDatabase& database);

}
}
}
#endif
