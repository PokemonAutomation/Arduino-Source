/*  VS Seeker Icon Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Right after the VS Seeker is used, every trainer in range shows an icon
 *  above their head for about half a second: "!!" if they want a rematch,
 *  "X" if not. Each icon is a 16x16 speech bubble drawn centered over the
 *  trainer's tile, its center 17 pixels above the tile's top edge.
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_VsSeekerIconDetector_H
#define PokemonAutomation_PokemonFRLG_VsSeekerIconDetector_H

#include <vector>

namespace PokemonAutomation{
    class ImageViewRGB32;
namespace NintendoSwitch{
namespace PokemonFRLG{


enum class VsSeekerIcon{
    NONE,
    READY,      //  "!!": white bubble, navy marks.
    NOT_READY,  //  "X": light blue bubble, white cross.
};

//  Look for either icon inside a region of the screen, given in GBA pixels.
//  If both are there, READY wins.
VsSeekerIcon find_vs_seeker_icon(const ImageViewRGB32& frame, int left, int top, int right, int bottom);

//  Every icon on the screen, with its center in GBA pixels.
struct VsSeekerIconSighting{
    VsSeekerIcon icon = VsSeekerIcon::NONE;
    int x = 0;
    int y = 0;
};
std::vector<VsSeekerIconSighting> find_vs_seeker_icons(const ImageViewRGB32& frame);



}
}
}
#endif
