/*  Battle Pokemon Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonTools/Images/SolidColorTest.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "PokemonFRLG/PokemonFRLG_Settings.h"
#include "PokemonFRLG_BattlePokemonDetector.h"

// #include <iostream>
// using std::cout;
// using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

BattlePokemonDetector::BattlePokemonDetector(Color color)
    // be warned: the name/level/hp box moves up and down a little bit
    // : m_left_box(0.583221, 0.566642, 0.021513, 0.071702) // off-white (255, 255, 236), can be interrupted by a status condition
    : m_right_box(0.943016, 0.585101, 0.010587, 0.061961) // dark teal (74, 111, 102)
    // , m_top_box(0.594712, 0.482692, 0.325962, 0.00600) // off-white, movement makes this unreliable
    , m_bottom_box(0.558121, 0.672085, 0.025232, 0.008631) // dark teal
{}
void BattlePokemonDetector::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    // items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_left_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_right_box));
    // items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_top_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_bottom_box));
}
bool BattlePokemonDetector::detect(const ImageViewRGB32& screen){
    ImageViewRGB32 game_screen = extract_box_reference(screen, GameSettings::instance().GAME_BOX);

    // ImageViewRGB32 left_image = extract_box_reference(game_screen, m_left_box);
    ImageViewRGB32 right_image = extract_box_reference(game_screen, m_right_box);
    // ImageViewRGB32 top_image = extract_box_reference(game_screen, m_top_box);
    ImageViewRGB32 bottom_image = extract_box_reference(game_screen, m_bottom_box);
    if ( // is_solid(left_image, { 0.3418, 0.3418, 0.3164 })
        is_solid(right_image, { 0.2578, 0.3868, 0.3554 })
        // && is_solid(top_image, { 0.3418, 0.3418, 0.3164 })
        && is_solid(bottom_image, { 0.2578, 0.3868, 0.3554 })
    ){
        return true;
    }
    return false;
}

BattleOpponentDetector::BattleOpponentDetector(Color color)
    : m_left_box(0.068132, 0.133653, 0.005495, 0.097253) // off-white (255, 255, 236)
    , m_right_box(0.415681, 0.123763, 0.003297, 0.064286) // off-white 
    // , m_top_box(0.079022, 0.113170, 0.336264, 0.004945) // off-white, unreliable (text shadow?)
    , m_bottom_box(0.114465, 0.262000, 0.327224, 0.007000) // dark teal (74, 111, 102)
{}
void BattleOpponentDetector::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_left_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_right_box));
    // items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_top_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_bottom_box));
}
bool BattleOpponentDetector::detect(const ImageViewRGB32& screen){
    ImageViewRGB32 game_screen = extract_box_reference(screen, GameSettings::instance().GAME_BOX);

    ImageViewRGB32 left_image = extract_box_reference(game_screen, m_left_box);
    ImageViewRGB32 right_image = extract_box_reference(game_screen, m_right_box);
    // ImageViewRGB32 top_image = extract_box_reference(game_screen, m_top_box);
    ImageViewRGB32 bottom_image = extract_box_reference(game_screen, m_bottom_box);

    if (is_solid(left_image, { 0.3418, 0.3418, 0.3164 })
        && is_solid(right_image, { 0.3418, 0.3418, 0.3164 })
        // && is_solid(top_image, { 0.3418, 0.3418, 0.3164 })
        && is_solid(bottom_image, { 0.2578, 0.3868, 0.3554 })
    ){
        return true;
    }
    return false;
}



}
}
}
