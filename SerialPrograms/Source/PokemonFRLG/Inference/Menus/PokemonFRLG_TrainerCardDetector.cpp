/*  Trainer Card Detector
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
#include "PokemonFRLG_TrainerCardDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

TrainerCardDetector::TrainerCardDetector(Color color)
    : m_box_tid_top(0.581250, 0.108413, 0.332692, 0.007212) // light blue (227, 255, 255)
    , m_box_tid_right(0.914904, 0.117067, 0.005769, 0.064904) 
    , m_box_stripe_top(0.038942, 0.090384, 0.918269, 0.002885) // blue  (94, 177, 255)
    , m_box_stripe_right(0.954808, 0.095432, 0.002885, 0.109615)
    , m_box_border_top(0.051442, 0.040625, 0.895192, 0.003606) // gray (100, 100, 120)
    , m_box_border_right(0.968750, 0.075961, 0.002885, 0.848077)
{}
void TrainerCardDetector::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_box_tid_top));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_box_tid_right));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_box_stripe_top));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_box_stripe_right));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_box_border_top));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_box_border_right));
}
bool TrainerCardDetector::detect(const ImageViewRGB32& screen){
    ImageViewRGB32 game_screen = extract_box_reference(screen, GameSettings::instance().GAME_BOX);

    ImageViewRGB32 image_tid_top = extract_box_reference(game_screen, m_box_tid_top);
    ImageViewRGB32 image_tid_right = extract_box_reference(game_screen, m_box_tid_right);
    ImageViewRGB32 image_stripe_top = extract_box_reference(game_screen, m_box_stripe_top);
    ImageViewRGB32 image_stripe_right = extract_box_reference(game_screen, m_box_stripe_right);
    ImageViewRGB32 image_border_top = extract_box_reference(game_screen, m_box_border_top);
    ImageViewRGB32 image_border_right = extract_box_reference(game_screen, m_box_border_right);

    if ( is_solid(image_tid_top, { 0.308, 0.346, 0.346 })
        && is_solid(image_tid_right, { 0.308, 0.346, 0.346 })
        && is_solid(image_stripe_top, { 0.179, 0.337, 0.485 })
        && is_solid(image_stripe_right, { 0.179, 0.337, 0.485 })
        && is_solid(image_border_top, { 0.313, 0.313, 0.375 })
        && is_solid(image_border_right, { 0.313, 0.313, 0.375 })
    ){
        return true;
    }
    return false;
}



}
}
}
