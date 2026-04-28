/*  Dex Registration Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "PokemonFRLG_DexRegistrationDetector.h"


namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


DexRegistrationDetector::DexRegistrationDetector(Color color)
    : m_top_box(0.940865, 0.013942, 0.049519, 0.071394)
    , m_header_box(0.037019, 0.147355, 0.928365, 0.015865)
    , m_divider_box(0.016827, 0.560577, 0.961827, 0.002885)
    , m_body_box(0.016827, 0.582577, 0.961827, 0.002885)
    , m_bottom_box(0.007692, 0.912500, 0.688942, 0.075721)
{}
void DexRegistrationDetector::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_top_box));     // Green RGB(189, 205,   0)
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_header_box));  // White
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_divider_box)); // Brown RGB(136, 100,  38)
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_body_box));    // Beige RGB(255, 245, 204)
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_bottom_box));  // Green

}
bool DexRegistrationDetector::detect(const ImageViewRGB32& screen){
    ImageViewRGB32 game_screen = extract_box_reference(screen, GameSettings::instance().GAME_BOX);

    ImageViewRGB32 top_image = extract_box_reference(game_screen, m_top_box);
    ImageViewRGB32 header_image = extract_box_reference(game_screen, m_header_box);
    ImageViewRGB32 divider_image = extract_box_reference(game_screen, m_divider_box);
    ImageViewRGB32 body_image = extract_box_reference(game_screen, m_body_box);
    ImageViewRGB32 bottom_image = extract_box_reference(game_screen, m_bottom_box);


    if (   is_solid(top_image,      { 0.480, 0.520, 0.000 }, 0.25, 20)
        && is_white(header_image)
        && is_solid(divider_image,  { 0.496, 0.365, 0.139 }, 0.25, 20)
        && is_solid(body_image,     { 0.362, 0.348, 0.290 }, 0.25, 20)
        && is_solid(bottom_image,   { 0.480, 0.520, 0.000 }, 0.25, 20)
        && !is_white(top_image)
        && !is_white(divider_image) 
        && !is_white(bottom_image)
    ){
        return true;
    }
    return false;
}



}
}
}
