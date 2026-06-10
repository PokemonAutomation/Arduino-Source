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
    : m_left_box(0.577404, 0.504327, 0.001442, 0.121875) // off-white rgb(255, 255, 235), can be interrupted by a status condition
    , m_right_box(0.948558, 0.585096, 0.000481, 0.063462) // dark teal rgb(72, 106, 98)
    , m_top_box(0.594712, 0.481971, 0.325962, 0.002163) // off-white, movement makes this unreliable
    , m_bottom_box(0.554808, 0.674519, 0.034615, 0.002163) // dark teal
    , m_background_box(0.552564, 0.583653, 0.414103, 0.009615) // depends on in-game location. white or rgb(215, 169, 96)
{}
void BattlePokemonDetector::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_left_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_right_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_top_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_bottom_box));
}
bool BattlePokemonDetector::detect(const ImageViewRGB32& screen){
    ImageViewRGB32 game_screen = extract_box_reference(screen, GameSettings::instance().GAME_BOX);

    ImageViewRGB32 left_image = extract_box_reference(game_screen, m_left_box);
    ImageViewRGB32 right_image = extract_box_reference(game_screen, m_right_box);
    ImageViewRGB32 top_image = extract_box_reference(game_screen, m_top_box);
    ImageViewRGB32 bottom_image = extract_box_reference(game_screen, m_bottom_box);
    ImageViewRGB32 background_image = extract_box_reference(game_screen, m_background_box);

    if (is_grey(left_image, 680, 770, 20)
        && is_grey(right_image, 220, 320, 20)
        && is_grey(top_image, 680, 770, 20)
        && is_grey(bottom_image, 220, 320, 20)
        && !(is_white(background_image) || is_solid(background_image, {0.4468, 0.3528, 0.2004}, 0.25, 20))
    ){
        return true;
    }
    return false;
}

BattleOpponentDetector::BattleOpponentDetector(Color color)
    : m_left_box(0.067308, 0.140865, 0.001442, 0.090144) // off-white rgb(255, 255, 235)
    , m_right_box(0.422596, 0.132211, 0.000481, 0.054808) // off-white 
    , m_top_box(0.085577, 0.119952, 0.329327, 0.000721) //  off-white
    , m_bottom_box(0.109615, 0.264182, 0.328365, 0.001442) // dark teal rgb(72, 106, 98)
    , m_background_box(0.050641, 0.126923, 0.388462, 0.063462) // depends on in-game location. white or rgb(187, 149, 65)
{}
void BattleOpponentDetector::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_left_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_right_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_top_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_bottom_box));
}
bool BattleOpponentDetector::detect(const ImageViewRGB32& screen){
    ImageViewRGB32 game_screen = extract_box_reference(screen, GameSettings::instance().GAME_BOX);

    ImageViewRGB32 left_image = extract_box_reference(game_screen, m_left_box);
    ImageViewRGB32 right_image = extract_box_reference(game_screen, m_right_box);
    ImageViewRGB32 top_image = extract_box_reference(game_screen, m_top_box);
    ImageViewRGB32 bottom_image = extract_box_reference(game_screen, m_bottom_box);
    ImageViewRGB32 background_image = extract_box_reference(game_screen, m_background_box);

    if (is_grey(left_image, 680, 770, 20)
        && is_grey(right_image, 680, 770, 20)
        && is_grey(top_image, 680, 770, 20)
        && is_grey(bottom_image, 220, 320, 20)
        && !(is_white(background_image) || is_solid(background_image, {0.4663, 0.3716, 0.1621}, 0.25, 20))
    ){
        return true;
    }
    return false;
}



}
}
}
