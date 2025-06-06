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

// return true if the area within the selected_box is highlighted, compared with the area within unselected_box
// This compares the brightness of the selected_box with the unselected_box.
// selected_box: the box where we expect the screen should be highlighted
// unselected_box 1 and 2: the boxes where we expect the screen should NOT be highlighted. These acts as the control, for comparison.
// the average sum of selected_box should be greater than the absolute difference of average sum between unselected_box 1 and 2.
bool is_setting_selected(VideoStream& stream, ProControllerContext& context, ImageFloatBox selected_box, ImageFloatBox unselected_box1, ImageFloatBox unselected_box2);

void home_to_date_time(VideoStream& stream, ProControllerContext& context, bool to_date_change, bool fast);
void home_to_date_time(ProControllerContext& context, bool to_date_change, bool fast);

//Joycon must not be sideways
void home_to_date_time(JoyconContext& context, bool to_date_change);



}
}
#endif
