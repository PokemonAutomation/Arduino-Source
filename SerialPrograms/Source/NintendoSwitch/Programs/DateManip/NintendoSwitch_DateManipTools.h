/*  Date Manipulation Tools
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_DateManipTools_H
#define PokemonAutomation_NintendoSwitch_DateManipTools_H

#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "NintendoSwitch_DateManipBase.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace DateReaderTools{



ImageRGB32 filter_image(const ImageViewRGB32& image);
int read_box(
    Logger& logger,
    int min, int max,
    const ImageViewRGB32& screen, const ImageFloatBox& box
);

void move_horizontal(ProControllerContext& context, int current, int desired);

void adjust_no_wrap_Switch1(ProControllerContext& context, int current, int desired);
void adjust_no_wrap_Switch2(ProControllerContext& context, int current, int desired);

void adjust_wrap_Switch1(ProControllerContext& context, int min, int max, int current, int desired);
void adjust_wrap_Switch2(ProControllerContext& context, int min, int max, int current, int desired);



}
}
}
#endif
