/*  Menu Cursor Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_MenuCursorDetector_H
#define PokemonAutomation_PokemonBDSP_MenuCursorDetector_H

#include <string>
#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"

namespace PokemonAutomation{
class UnitTestDatabase;
class VideoOverlaySet;

namespace NintendoSwitch{
namespace PokemonBDSP{


enum class MenuCursorPosition{
    UNKNOWN = -1,
    POKEDEX = 0,
    POKEMON = 1,
    BAG = 2,
    CARD = 3,
    OPTIONS = 4,
    MAP = 5,
    CAPSULES = 6,
    MYSTERYGIFT = 7,
};

std::string menu_cursor_to_string(MenuCursorPosition pos);

// Identifies the selected X-menu icon in the default story unlock order.
// User-reordered icons are not supported. Uses relative grayscale brightness
// at two selection borders, not a fixed dark RGB threshold or icon templates.
// Grayscale reduces sensitivity to tint; clipping and heavy blur can still
// erase the border contrast. Callers should wait for a stable menu frame.
class MenuCursorDetector{
public:
    explicit MenuCursorDetector(Color color = COLOR_RED);
    void make_overlays(VideoOverlaySet& items) const;

    // Returns UNKNOWN outside the menu or when no unique selection is visible.
    // On success, icon_count receives the visible layout size (2-8); otherwise 0.
    MenuCursorPosition detect(const ImageViewRGB32& screen, int* icon_count = nullptr) const;

private:
    Color m_color;
    ImageFloatBox m_icons[8];
};

void add_tests_MenuCursorDetector(UnitTestDatabase& database);

}
}
}
#endif
