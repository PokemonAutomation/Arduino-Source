/*  Terarium
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_Terarium_H
#define PokemonAutomation_PokemonSV_Terarium_H

#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"

namespace PokemonAutomation{
    struct ProgramInfo;
namespace NintendoSwitch{
namespace PokemonSV{

// Return to Central Plaza from anywhere in the terarium
void return_to_plaza(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context);

// Open map from overworld, move cursor to fly location, and fly to location
void map_move_cursor_fly(
    const ProgramInfo& info,
    VideoStream& stream, ProControllerContext& context,
    uint8_t x, uint8_t y,
    uint8_t hold, uint8_t release,
    std::string location
);

// From central plaza, fly to the polar rest area
void central_to_polar_rest(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context);

// From central plaza, fly to polar outdoor classroom 1
void central_to_polar_class1(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context);

// From central plaza, fly to coastal plaza
void central_to_polar_plaza(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context);

// From central plaza, fly to coastal plaza
void central_to_coastal_plaza(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context);

// From central plaza, fly to canyon plaza
void central_to_canyon_plaza(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context);

// From central plaza, fly to savanna plaza
void central_to_savanna_plaza(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context);

// From central plaza, fly to canyon rest area
void central_to_canyon_rest(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context);

// From central plaza, fly to savanna classroom. Warning: extremely laggy area.
void central_to_savanna_class(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context);

// From central plaza, fly to chargestone cavern
void central_to_chargestone(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context);

// Jump straight up into the air and fly. Fly up for hold_up ticks, fly straight for flight_wait ticks, and wait drop_time ticks to fall to the ground.
void jump_glide_fly(
    VideoStream& stream, ProControllerContext& context,
    bool inverted_flight,
    uint16_t hold_up,
    uint16_t flight_wait,
    uint16_t drop_time
);


}
}
}
#endif
