/*  Game Navigation
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_GameNavigation_H
#define PokemonAutomation_PokemonBDSP_GameNavigation_H

#include <functional>
#include "CommonFramework/Tools/VideoStream.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


//  Non-Feedback

void save_game(ProControllerContext& context);

void menu_to_box(ProControllerContext& context);
void overworld_to_box(ProControllerContext& context);
void box_to_overworld(ProControllerContext& context);


//  Feedback

// Wait for three consecutive detections on fresh captures after queued input drains.
// On timeout, log and save the last checked frame before the caller retries or backs out.
// last_checked also lets an exception reuse that exact frame rather than a later capture.
bool wait_for_menu_screen(VideoStream& stream, ProControllerContext& context,
    const std::function<bool(const ImageViewRGB32&)>& detect, const std::string& label,
    Milliseconds timeout = Milliseconds(5000), VideoSnapshot* last_checked = nullptr);

void overworld_to_menu(VideoStream& stream, ProControllerContext& context);
void save_game(VideoStream& stream, ProControllerContext& context);

void overworld_to_box(VideoStream& stream, ProControllerContext& context);
void box_to_overworld(VideoStream& stream, ProControllerContext& context);


}
}
}
#endif
