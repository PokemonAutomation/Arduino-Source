/*  Nintendo Switch Home To Date-Time
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_HomeToDateTime_H
#define PokemonAutomation_NintendoSwitch_HomeToDateTime_H

//#include "CommonFramework/ImageTools/ImageBoxes.h"
//#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_Joycon.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


//// Navigates from Home screen to the Date and Time screen. Using visual inference.
//void home_to_date_time(VideoStream& stream, ProControllerContext& context, bool to_date_change);

//// Navigates from Home screen to the Date and Time screen. Done blind, without inference.
//void home_to_date_time(Logger& logger, ProControllerContext& context, bool to_date_change);


void home_to_date_time_Switch1_wired_blind(
    Logger& logger, ProControllerContext& context, bool to_date_change
);
void home_to_date_time_Switch1_wireless_esp32_blind(
    Logger& logger, ProControllerContext& context, bool to_date_change
);
void home_to_date_time_Switch1_sbb_blind(
    Logger& logger, ProControllerContext& context, bool to_date_change
);
void home_to_date_time_Switch1_wired_feedback(
    VideoStream& stream, ProControllerContext& context, bool to_date_change
);



void home_to_date_time_Switch2_wired_blind(
    Logger& logger, ProControllerContext& context,
    ConsoleType console_type, bool to_date_change
);
void home_to_date_time_Switch2_wired_feedback(
    ConsoleHandle& console, ProControllerContext& context,
    bool to_date_change
);



void home_to_date_time(ConsoleHandle& console, ProControllerContext& context, bool to_date_change);



//Joycon must not be sideways
void home_to_date_time(JoyconContext& context, bool to_date_change);



}
}
#endif
