/*  Pokemon Home Box Spaces Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonHome_BoxSpacesReader_H
#define PokemonAutomation_PokemonHome_BoxSpacesReader_H

#include <cstddef>
#include <optional>
#include "Common/Cpp/Color.h"

namespace PokemonAutomation{
    class ImageViewRGB32;
    class UnitTestDatabase;
    class VideoOverlaySet;
namespace NintendoSwitch{
namespace PokemonHome{

class BoxSpacesReader{
public:
    BoxSpacesReader(Color color = COLOR_RED) : m_color(color) {}

    void make_overlays(VideoOverlaySet& items) const;

    // Read the one-based page number from the Box Space title.
    // Returns no value when the page number is unavailable.
    std::optional<size_t> read_page_number(const ImageViewRGB32& screen) const;

private:
    Color m_color;
};

void add_tests_BoxSpacesReader(UnitTestDatabase& database);

}
}
}
#endif
