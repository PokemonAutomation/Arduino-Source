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

// return true if the Switch Settings uses White theme
// assumes we are currently in the Switch Settings.
bool is_white_theme(VideoStream& stream, ProControllerContext& context);


bool is_setting_selected(VideoStream& stream, ProControllerContext& context, ImageFloatBox selected_box, ImageFloatBox unselected_box1, ImageFloatBox unselected_box2);

void home_to_date_time(VideoStream& stream, ProControllerContext& context, bool to_date_change, bool fast);
void home_to_date_time(ProControllerContext& context, bool to_date_change, bool fast);

//Joycon must not be sideways
void home_to_date_time(JoyconContext& context, bool to_date_change);



}
}
#endif
