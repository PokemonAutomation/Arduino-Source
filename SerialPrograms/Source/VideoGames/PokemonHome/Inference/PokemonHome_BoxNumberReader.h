/*  Pokemon Home Box Number Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonHome_BoxNumberReader_H
#define PokemonAutomation_PokemonHome_BoxNumberReader_H

#include <optional>
#include "Common/Cpp/Color.h"

namespace PokemonAutomation{
    class ImageViewRGB32;
    class UnitTestDatabase;
    class VideoOverlaySet;
namespace NintendoSwitch{
namespace PokemonHome{

class BoxNumberReader{
public:
    BoxNumberReader(Color color = COLOR_RED) : m_color(color) {}

    void make_overlays(VideoOverlaySet& items) const;

    // Read the one-based box number from the lower-left counter on the box screen.
    // Returns no value if the counter is missing or unreadable.
    std::optional<size_t> read_box_number(const ImageViewRGB32& screen) const;

private:
    Color m_color;
};

void add_tests_BoxNumberReader(UnitTestDatabase& database);

}
}
}
#endif
