/*  Pokemon Home Storage View
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonHome_StorageView_H
#define PokemonAutomation_PokemonHome_StorageView_H

#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{

// These locations are the same for both Box View and Page View.
inline const ImageFloatBox STORAGE_HEADER_STRIP(0.17, 0.091, 0.16, 0.004);
inline const ImageFloatBox STORAGE_PLUS_BUTTON(0.100, 0.940, 0.107, 0.057);

enum class StorageViewHeader{
    Unknown,
    BoxView,
    PageView,
};

inline StorageViewHeader classify_storage_header(const ImageViewRGB32& screen){
    const ImageViewRGB32 header = extract_box_reference(screen, STORAGE_HEADER_STRIP);
    if (!header){
        return StorageViewHeader::Unknown;
    }
    const FloatPixel color = image_average(header);
    // Both headers are teal. Page View has a stronger blue channel.
    if (color.g <= color.r + 80){
        return StorageViewHeader::Unknown;
    }
    if (color.b > color.g + 15){
        return StorageViewHeader::PageView;
    }
    if (color.b < color.g + 10){
        return StorageViewHeader::BoxView;
    }
    return StorageViewHeader::Unknown;
}

}
}
}
#endif
