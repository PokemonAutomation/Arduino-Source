/*  Nintendo Switch Navigation
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_Navigation_H
#define PokemonAutomation_NintendoSwitch_Navigation_H

#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_Joycon.h"

namespace PokemonAutomation{
namespace NintendoSwitch{

void home_to_date_time_Switch1_wired_with_feedback(VideoStream& stream, ProControllerContext& context, bool to_date_change);

void home_to_date_time_Switch1_wired_blind(ProControllerContext& context, bool to_date_change);
void home_to_date_time_Switch1_wireless_esp32_blind(ProControllerContext& context, bool to_date_change);
void home_to_date_time_Switch1_sbb_blind(ProControllerContext& context, bool to_date_change);

void home_to_date_time_Switch2_wired_blind(ProControllerContext& context, bool to_date_change);

// Navigates from Home screen to the Date and Time screen. Using visual inference.
void home_to_date_time(VideoStream& stream, ProControllerContext& context, bool to_date_change);
// Navigates from Home screen to the Date and Time screen. Done blind, without inference.
void home_to_date_time(ProControllerContext& context, bool to_date_change, bool fast);

//Joycon must not be sideways
void home_to_date_time(JoyconContext& context, bool to_date_change);



}
}
#endif
