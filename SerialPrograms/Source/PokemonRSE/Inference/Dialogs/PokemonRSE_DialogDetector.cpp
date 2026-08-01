/*  Dialog Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonTools/Images/SolidColorTest.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "PokemonRSE/PokemonRSE_Settings.h"
#include "PokemonRSE_DialogDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonRSE{



PokenavDialogDetector::PokenavDialogDetector(Color color)
    : m_dialog_border_box(0.04, 0.717, 0.926425, 0.002033) //Very narrow, not many places to detect
    , m_dialog_main_box(0.039784, 0.759311, 0.039008, 0.010672)
{}
void PokenavDialogDetector::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_dialog_border_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_dialog_main_box));
}
bool PokenavDialogDetector::detect(const ImageViewRGB32& screen){
    ImageViewRGB32 game_screen = extract_box_reference(screen, GameSettings::instance().GAME_BOX);

    ImageViewRGB32 dialog_border_image = extract_box_reference(game_screen, m_dialog_border_box);
    ImageViewRGB32 dialog_main_image = extract_box_reference(game_screen, m_dialog_main_box);

    if (is_solid(dialog_border_image, { 0.20498, 0.45, 0.3448 }, 0.25, 20) //mint green 107, 235, 180
        && is_white(dialog_main_image)
    ){
        return true;
    }
    return false;
}




}
}
}
