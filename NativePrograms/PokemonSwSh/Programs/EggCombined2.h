/*  Pokemon Sword & Shield Arduino Programs
 * 
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 * 
 */

#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"


extern const uint8_t   BOXES_TO_HATCH;
extern const uint16_t  STEPS_TO_HATCH;      //  Lookup the # of steps on Serebii.


//  For each batch of eggs, attempt this many egg fetches.
//
//  This parameter works by dividing the incubation time by the # of fetch
//  attempts and doing a fetch at this interval. Therefore increasing this
//  number will decrease the time between fetches which also decreases the
//  chance of successfully fetching an egg. The optimal value is difficult to
//  determine and is dependent on many factors.
//
//  For fast hatching eggs like Magikarp, you may need to decrease this number
//  to as low as 1 or 2. This implies that it's impossible for fetching to keep
//  pace with hatching as is the case with manual hatching anyway.
//
//  The program will automatically reduce this number if it determines that it
//  is too large to function correctly.
//
//  If this is set to a non-integer, the program will vary the # of fetches
//  across batches so that they average out to the specified number.
//
extern const float FETCHES_PER_BATCH;



////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//  Advanced Settings (you shouldn't need to change these)

//  Additional time added to the spinning. If any shinies are hatched, they will
//  eat into this safety buffer along with any other unexpected slowdowns.
//  Hatching a shiny takes 2 seconds longer than a non-shiny.
//
//  If you see that the program is going into Y-COMM or there is less than 5
//  seconds of extra spinning after the last egg in the batch, please report
//  this as a bug. As a temporary work-around, you can increase this number.
extern const uint16_t SAFETY_TIME;

//  Eggs will not hatch early by more than this interval.
extern const uint16_t EARLY_HATCH_SAFETY;

//  Total animation time for hatching 5 eggs when there are no shinies.
extern const uint16_t HATCH_DELAY;


//  Rollover Prevention
//
//  Prevent a den from rolling over by periodically touching the date.
//
//  If set to zero, this feature is disabled.
//  Otherwise, the program will touch the date at roughly this interval to
//  prevent a den from rolling over. This feature requres time-sync be disabled.
extern const uint32_t TOUCH_DATE_INTERVAL;


