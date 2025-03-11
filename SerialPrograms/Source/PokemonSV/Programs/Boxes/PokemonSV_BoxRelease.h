/*  Box Release
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_BoxRelease_H
#define PokemonAutomation_PokemonSV_BoxRelease_H

#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"

namespace PokemonAutomation{
    struct ProgramInfo;
namespace NintendoSwitch{
namespace PokemonSV{


//  Assuming the current slot in box system is not an egg
//  release current selected pokemon in the box system.
//  It will do nothing if the current slot is empty.
//  Throws OperationFailedException, if it got stuck or timed out.
//  The # of errors are stored into "errors". These are usually dropped button
//  presses that the function recovered from.
void release_one_pokemon(
    const ProgramInfo& info,
    VideoStream& stream, ProControllerContext& context,
    size_t& errors
);

//  Release a box of pokemon. Can have empty spots or eggs. Eggs are not released.
//  Throws OperationFailedException, if it got stuck or timed out.
//  The # of errors are stored into "errors". These are usually dropped button
//  presses that the function recovered from.
void release_box(
    const ProgramInfo& info,
    VideoStream& stream, ProControllerContext& context,
    size_t& errors,
    uint8_t start_row = 0   //  Start from this row. (skip te first "start_row" rows)
);


}
}
}
#endif
