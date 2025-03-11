/*  Box Routines
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Various functions to operate in box system.
 */

#ifndef PokemonAutomation_PokemonSV_BoxRoutines_H
#define PokemonAutomation_PokemonSV_BoxRoutines_H

#include <stdint.h>
#include "CommonFramework/Language.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"

namespace PokemonAutomation{
    
    struct ProgramInfo;
    class ProgramEnvironment;
    class EventNotificationOption;

namespace NintendoSwitch{
namespace PokemonSV{

enum class BoxCursorLocation;

//  Assuming the current slot in box system is a pokemon, not egg or empty space,
//  try to change the view to the judge. However, it may land on the stats instead.
//  If it can't land on either stats or judge and `throw_exception` is true, it will throw an exception.
//  Return true if it successfully changed view to judge (or stats if judege is not found)
bool change_view_to_stats_or_judge(
    VideoStream& stream, ProControllerContext& context,
    bool throw_exception = true
);


//  Assuming the current slot in box system is a pokemon, not egg or empty space,
//  change the view to the judge. If it fails, it will OperationFailedException::fire.
void change_view_to_judge(
    VideoStream& stream, ProControllerContext& context,
    Language language
);


//  Press button L to move to the box on the left
void move_to_left_box(ProControllerContext& context);

//  Press button R to move to the box on the right
void move_to_right_box(ProControllerContext& context);



//  In box system, check how many slots in the party are empty
uint8_t check_empty_slots_in_party(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context);

//  In box system, assuming the party is empty, load one column in the current box onto party.
//  if has_clone_ride_pokemon is true, skip loading the first row of the box.
void load_one_column_to_party(
    ProgramEnvironment& env, VideoStream& stream, ProControllerContext& context,
    EventNotificationOption& notification,
    uint8_t column_index, bool has_clone_ride_pokemon
);

//  In box system, assuming the target column is empty, unload party (after the lead) to the target column.
//  if has_clone_ride_pokemon is true, skip unloading the second row of the party.
void unload_one_column_from_party(
    ProgramEnvironment& env, VideoStream& stream, ProControllerContext& context,
    EventNotificationOption& notification,
    uint8_t column_index, bool has_clone_ride_pokemon
);

//  In box system, move the cursor to the desired slot.
void move_box_cursor(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context,
    const BoxCursorLocation& side, uint8_t row, uint8_t col);

//  In box system, use button Y to swap two slots.
//  The source slot must not be empty, while the target slot can be empty.
void swap_two_box_slots(
    const ProgramInfo& info, VideoStream& stream, ProControllerContext& context,
    const BoxCursorLocation& source_side, uint8_t source_row, uint8_t source_col,
    const BoxCursorLocation& target_side, uint8_t target_row, uint8_t target_col
);

}
}
}
#endif
