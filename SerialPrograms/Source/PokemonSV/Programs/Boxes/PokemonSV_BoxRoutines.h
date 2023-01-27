/*  Box Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *  Various functions to operate in box system, like release pokemon.
 */

#ifndef PokemonAutomation_PokemonSV_BoxRoutines_H
#define PokemonAutomation_PokemonSV_BoxRoutines_H

#include <stdint.h>
#include "CommonFramework/Language.h"

namespace PokemonAutomation{
    
    struct ProgramInfo;
    class ProgramEnvironment;
    class EventNotificationOption;
    class ConsoleHandle;
    class BotBaseContext;

namespace NintendoSwitch{
namespace PokemonSV{

enum class BoxCursorLocation;

//  Assuming the current slot in box system is a pokemon, not egg or empty space,
//  try to change the view to the judge. However, it may land on the stats instead.
//  If it can't land on either stats or judge and `throw_exception` is true, it will throw an exception.
//  Return true if it successfully changed view to judge (or stats if judege is not found)
bool change_view_to_stats_or_judge(
    ConsoleHandle& console, BotBaseContext& context,
    bool throw_exception = true
);


//  Assuming the current slot in box system is a pokemon, not egg or empty space,
//  change the view to the judge. If it fails, it will throw OperationFailedException.
void change_view_to_judge(
    ConsoleHandle& console, BotBaseContext& context,
    Language language
);


//  Press button L to move to the box on the left
void move_to_left_box(BotBaseContext& context);

//  Press button R to move to the box on the right
void move_to_right_box(BotBaseContext& context);

//  Assuming the current slot in box system is not an egg
//  release current selected pokemon in the box system.
//  It will do nothing if the current slot is empty. 
//  Throws OperationFailedException, if it got stuck or timed out.
//  The # of errors are stored into "errors". These are usually dropped button
//  presses that the function recovered from.
void release_one_pokemon(
    const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context,
    size_t& errors
);

//  Release a box of pokemon. Can have empty spots or eggs. Eggs are not released.
//  Throws OperationFailedException, if it got stuck or timed out.
//  The # of errors are stored into "errors". These are usually dropped button
//  presses that the function recovered from.
void release_box(
    const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context,
    size_t& errors,
    uint8_t start_row = 0   //  Start from this row. (skip te first "start_row" rows)
);

//  In box system, check how many slots in the party are empty
uint8_t check_empty_slots_in_party(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context);

//  In box system, assuming the party is empty, load one column in the current box onto party.
//  if has_clone_ride_pokemon is true, skip loading the first row of the box.
void load_one_column_to_party(
    ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context,
    EventNotificationOption& notification,
    uint8_t column_index, bool has_clone_ride_pokemon
);

//  In box system, assuming the target column is empty, unload party (after the lead) to the target column.
//  if has_clone_ride_pokemon is true, skip unloading the second row of the party.
void unload_one_column_from_party(
    ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context,
    EventNotificationOption& notification,
    uint8_t column_index, bool has_clone_ride_pokemon
);

//  In box system, move the cursor to the desired slot.
void move_box_cursor(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context,
    const BoxCursorLocation& side, uint8_t row, uint8_t col);

//  In box system, use button Y to swap two slots.
//  The source slot must not be empty, while the target slot can be empty.
void swap_two_box_slots(
    const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context,
    const BoxCursorLocation& source_side, uint8_t source_row, uint8_t source_col,
    const BoxCursorLocation& target_side, uint8_t target_row, uint8_t target_col
);

}
}
}
#endif
