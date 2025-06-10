/*  PokemonSV Menu Navigation
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Move between different game menu states: overworld, menu, box system, etc.
 *  The location of the player charater in the overworld is not moved
 */

#ifndef PokemonAutomation_PokemonSV_MenuNavigation_H
#define PokemonAutomation_PokemonSV_MenuNavigation_H

#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"
#include "PokemonSV/Inference/PokemonSV_MainMenuDetector.h"


namespace PokemonAutomation{
    struct ProgramInfo;
namespace NintendoSwitch{
namespace PokemonSV{


void set_time_to_12am_from_home(const ProgramInfo& info, ConsoleHandle& console, ProControllerContext& context);

//  Perform a No-op day skip that rolls over all the outbreaks and raids.
void day_skip_from_overworld(ConsoleHandle& console, ProControllerContext& context);

// Press B to return to the overworld
void press_Bs_to_back_to_overworld(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context,
    uint16_t seconds_between_b_presses = 3);

// mashes A button by default
void mash_button_till_overworld(
    VideoStream& stream,
    ProControllerContext& context,
    Button button = BUTTON_A,
    uint16_t seconds_run = 360
);

//  From overworld, open map. Will change map view from rotated to fixed if not already fixed.
void open_map_from_overworld(
    const ProgramInfo& info,
    VideoStream& stream,
    ProControllerContext& context,
    bool clear_tutorial = false
);

//  Enter box system from overworld.
void enter_box_system_from_overworld(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context);

//  From box system go to overworld.
void leave_box_system_to_overworld(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context);

//  From overworld, open Pokédex.
void open_pokedex_from_overworld(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context);

//  From Pokédex, open Recently Battled.
void open_recently_battled_from_pokedex(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context);

//  From any of the rotom phone apps (Map/Pokédex/Profile) go to overworld.
void leave_phone_to_overworld(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context);

// enter menu and move the cursor to the given side and index, then press the A button (without wait for A button to press)
// if menu_index is -1, return once the menu is detected.
void enter_menu_from_overworld(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context,
    int menu_index,
    MenuSide side = MenuSide::RIGHT,
    bool has_minimap = true
);

// enter menu from box system and move the cursor to the given side and index, then press the A button (without wait for A
// button to press)
// if menu_index is -1, return once the menu is detected.
void enter_menu_from_box_system(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context,
    int menu_index,
    MenuSide side = MenuSide::NONE
);

// enter menu from bag and move the cursor to the given side and index, then press the A button (without wait for A
// button to press)
// if menu_index is -1, return once the menu is detected.
void enter_menu_from_bag(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context,
    int menu_index,
    MenuSide side = MenuSide::NONE
);

// enter bag from any place on the main menu
void enter_bag_from_menu(const ProgramInfo& info, VideoStream& stream, ProControllerContext& context);

// press given button until gradient arrow appears in given box_area_to_check.
void press_button_until_gradient_arrow(
    const ProgramInfo& info, 
    VideoStream& stream,
    ProControllerContext& context,
    ImageFloatBox box_area_to_check,
    Button button = BUTTON_A,
    GradientArrowType arrow_type = GradientArrowType::RIGHT
);

// navigate school layout menu using only gradient arrow detection, without OCR
// first, wait for the gradient arrow to appear within `arrow_box_start`.
// then, press `dpad_button` a certain number of times, as per `num_button_presses`.
// then, watch for the gradient arrow within `arrow_box_end`. 
// If arrow not seen, press `dpad_button` a maximum of 3 times, while still watching for the gradient arrow
// If arrow still not seen, throw exception.
void navigate_school_layout_menu(
    const ProgramInfo& info, 
    VideoStream& stream,
    ProControllerContext& context,
    ImageFloatBox arrow_box_start,
    ImageFloatBox arrow_box_end,
    DpadPosition dpad_button,
    uint16_t num_button_presses
);


}
}
}
#endif
