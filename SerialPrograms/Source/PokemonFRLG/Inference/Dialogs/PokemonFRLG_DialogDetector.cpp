/*  Dialog Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonTools/Images/WaterfillUtilities.h"
//#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
//#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "PokemonFRLG_DialogDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

WhiteDialogDetector::WhiteDialogDetector(Color color)
    : m_right_box(0.923385, 0.748077, 0.00615385, 0.204577)
    , m_top_box(0.0704615, 0.741846, 0.859077, 0.00623077)
    , m_bottom_box(0.0716923, 0.943308, 0.851692, 0.00934615)
{}
void WhiteDialogDetector::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_right_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_top_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_bottom_box));
}
bool WhiteDialogDetector::detect(const ImageViewRGB32& screen){
    ImageViewRGB32 game_screen = extract_box_reference(screen, GameSettings::instance().GAME_BOX);

    ImageViewRGB32 right_image = extract_box_reference(game_screen, m_right_box);
    ImageViewRGB32 top_image = extract_box_reference(game_screen, m_top_box);
    ImageViewRGB32 bottom_image = extract_box_reference(game_screen, m_bottom_box);
    if (is_white(right_image)
        && is_white(top_image)
        && is_white(bottom_image)
    ){
        return true;
    }
    return false;
}

AdvanceWhiteDialogDetector::AdvanceWhiteDialogDetector(Color color)
    : m_dialog_box(0.0630769, 0.750154, 0.870154, 0.200423)
    , m_right_box(0.923385, 0.748077, 0.00615385, 0.204577)
    , m_top_box(0.0704615, 0.741846, 0.859077, 0.00623077)
    , m_bottom_box(0.0716923, 0.943308, 0.851692, 0.00934615)
{}
void AdvanceWhiteDialogDetector::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_dialog_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_right_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_top_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_bottom_box));
}
bool AdvanceWhiteDialogDetector::detect(const ImageViewRGB32& screen){
    ImageViewRGB32 game_screen = extract_box_reference(screen, GameSettings::instance().GAME_BOX);

    const bool replace_color_within_range = false;

    //Filter out background
    ImageRGB32 filtered_region = filter_rgb32_range(
        extract_box_reference(game_screen, m_dialog_box),
        combine_rgb(164, 0, 0), combine_rgb(255, 114, 87), Color(0), replace_color_within_range
    );
    ImageStats stats = image_stats(filtered_region);

    /*
    filtered_region.save("./filtered_only.png");
    cout << stats.average.r << endl;
    cout << stats.average.g << endl;
    cout << stats.average.b << endl;
    */

    ImageViewRGB32 right_image = extract_box_reference(game_screen, m_right_box);
    ImageViewRGB32 top_image = extract_box_reference(game_screen, m_top_box);
    ImageViewRGB32 bottom_image = extract_box_reference(game_screen, m_bottom_box);

    if (is_white(right_image)
        && is_white(top_image)
        && is_white(bottom_image)
        && (stats.average.r > stats.average.b + 180)
        && (stats.average.r > stats.average.g + 180)
    ){
        return true;
    }
    return false;
}

SelectionDialogDetector::SelectionDialogDetector(Color color)
    : m_right_box(0.923385, 0.748077, 0.00615385, 0.204577)
    , m_top_box(0.0704615, 0.741846, 0.859077, 0.00623077)
    , m_bottom_box(0.0716923, 0.943308, 0.851692, 0.00934615)
    , m_selection_box(0.885231, 0.447962, 0.016, 0.2025)
{}
void SelectionDialogDetector::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_right_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_top_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_bottom_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_selection_box));
}
bool SelectionDialogDetector::detect(const ImageViewRGB32& screen){
    ImageViewRGB32 game_screen = extract_box_reference(screen, GameSettings::instance().GAME_BOX);

    ImageViewRGB32 right_image = extract_box_reference(game_screen, m_right_box);
    ImageViewRGB32 top_image = extract_box_reference(game_screen, m_top_box);
    ImageViewRGB32 bottom_image = extract_box_reference(game_screen, m_bottom_box);
    ImageViewRGB32 selection_image = extract_box_reference(game_screen, m_selection_box);
    if (is_white(right_image)
        && is_white(top_image)
        && is_white(bottom_image)
        && is_white(selection_image)
    ){
        return true;
    }
    return false;
}


BattleLearnDialogDetector::BattleLearnDialogDetector(Color color)
    : m_menu_top_box(0.805, 0.445, 0.149, 0.006)
    , m_menu_right_box(0.962, 0.445, 0.006, 0.200) // right side, Yes/No selection
    , m_dialog_top_box(0.036, 0.763, 0.459, 0.014)
    , m_dialog_right_box(0.941, 0.763, 0.006, 0.179) //right side, closest to the menu
{}
void BattleLearnDialogDetector::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_menu_top_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_menu_right_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_dialog_top_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_dialog_right_box));
}
bool BattleLearnDialogDetector::detect(const ImageViewRGB32& screen){
    ImageViewRGB32 game_screen = extract_box_reference(screen, GameSettings::instance().GAME_BOX);

    //Menu is white
    ImageViewRGB32 menu_top_image = extract_box_reference(game_screen, m_menu_top_box);
    ImageViewRGB32 menu_right_image = extract_box_reference(game_screen, m_menu_right_box);

    //Background dialog is teal
    ImageViewRGB32 dialog_top_image = extract_box_reference(game_screen, m_dialog_top_box);
    ImageViewRGB32 dialog_right_image = extract_box_reference(game_screen, m_dialog_right_box);

    if (is_white(menu_top_image)
        && is_white(menu_right_image)
        && is_solid(dialog_top_image, { 0.176, 0.357, 0.467 }, 0.25, 20) //40, 81, 106 teal
        && is_solid(dialog_right_image, { 0.176, 0.357, 0.467 }, 0.25, 20)
    ){
        return true;
    }
    return false;
}

PartyMenuDetector::PartyMenuDetector(Color color)
    : m_dialog_top_box(0.028, 0.840, 0.705, 0.010)
    , m_page_background_box(0.028, 0.500, 0.010, 0.250)
{}
void PartyMenuDetector::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_dialog_top_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_page_background_box));
}
bool PartyMenuDetector::detect(const ImageViewRGB32& screen){
    ImageViewRGB32 game_screen = extract_box_reference(screen, GameSettings::instance().GAME_BOX);

    //Dialog is white
    ImageViewRGB32 dialog_top_image = extract_box_reference(game_screen, m_dialog_top_box);

    //Menu background is teal
    ImageViewRGB32 menu_background_image = extract_box_reference(game_screen, m_page_background_box);

    if (is_white(dialog_top_image)
        && is_solid(menu_background_image, { 0.020, 0.424, 0.412 }, 0.25, 20) //5, 108, 105 teal
    ){
        return true;
    }
    return false;
}

PartySelectionDetector::PartySelectionDetector(Color color)
    : m_dialog_right_box(0.955, 0.648, 0.010, 0.303)
    , m_page_background_box(0.028, 0.500, 0.010, 0.250)
{}
void PartySelectionDetector::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_dialog_right_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_page_background_box));
}
bool PartySelectionDetector::detect(const ImageViewRGB32& screen){
    ImageViewRGB32 game_screen = extract_box_reference(screen, GameSettings::instance().GAME_BOX);

    //Dialog is white
    ImageViewRGB32 dialog_right_image = extract_box_reference(game_screen, m_dialog_right_box);

    //Menu background is teal
    ImageViewRGB32 menu_background_image = extract_box_reference(game_screen, m_page_background_box);

    if (is_white(dialog_right_image)
        && is_solid(menu_background_image, { 0.020, 0.424, 0.412 }, 0.25, 20) //5, 108, 105 teal
    ){
        return true;
    }
    return false;
}


}
}
}
