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
#include "CommonTools/Images/SolidColorTest.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "PokemonFRLG_DialogDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

WhiteDialogDetector::WhiteDialogDetector(Color color)
    : m_right_box(0.812, 0.726, 0.013, 0.169)
    , m_top_box(0.175, 0.715, 0.649, 0.005)
    , m_bottom_box(0.177, 0.896, 0.645, 0.008)
{}
void WhiteDialogDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_right_box);
    items.add(COLOR_RED, m_top_box);
    items.add(COLOR_RED, m_bottom_box);
}
bool WhiteDialogDetector::detect(const ImageViewRGB32& screen){
    ImageViewRGB32 right_image = extract_box_reference(screen, m_right_box);
    ImageViewRGB32 top_image = extract_box_reference(screen, m_top_box);
    ImageViewRGB32 bottom_image = extract_box_reference(screen, m_bottom_box);
    if (is_solid(right_image, { 0.25, 0.38, 0.369 })
        && is_solid(top_image, { 0.25, 0.38, 0.369 })
        && is_solid(bottom_image, { 0.25, 0.38, 0.369 })
        ){
        return true;
    }
    return false;
}

AdvanceWhiteDialogDetector::AdvanceWhiteDialogDetector(Color color)
    : m_dialog_box(0.170, 0.726, 0.655, 0.172)
    , m_right_box(0.812, 0.726, 0.013, 0.169)
    , m_top_box(0.175, 0.715, 0.649, 0.005)
    , m_bottom_box(0.177, 0.896, 0.645, 0.008)
{}
void AdvanceWhiteDialogDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_dialog_box);
    items.add(COLOR_RED, m_right_box);
    items.add(COLOR_RED, m_top_box);
    items.add(COLOR_RED, m_bottom_box);
}
bool AdvanceWhiteDialogDetector::detect(const ImageViewRGB32& screen){
    const bool replace_color_within_range = false;

    //Filter out background
    ImageRGB32 filtered_region = filter_rgb32_range(
        extract_box_reference(screen, m_dialog_box),
        combine_rgb(185, 0, 1), combine_rgb(255, 32, 33), Color(0), replace_color_within_range
    );
    ImageStats stats = image_stats(filtered_region);

    /*
    filtered_region.save("./filtered_only.png");
    cout << stats.average.r << endl;
    cout << stats.average.g << endl;
    cout << stats.average.b << endl;
    */

    ImageViewRGB32 right_image = extract_box_reference(screen, m_right_box);
    ImageViewRGB32 top_image = extract_box_reference(screen, m_top_box);
    ImageViewRGB32 bottom_image = extract_box_reference(screen, m_bottom_box);

    if (is_solid(right_image, { 0.25, 0.38, 0.369 })
        && is_solid(top_image, { 0.25, 0.38, 0.369 })
        && is_solid(bottom_image, { 0.25, 0.38, 0.369 })
        && (stats.average.r > stats.average.b + 200)
        && (stats.average.r > stats.average.g + 200)
        )
    {
        return true;
    }
    return false;
}

SelectionDialogDetector::SelectionDialogDetector(Color color)
    : m_right_box(0.812, 0.726, 0.013, 0.169)
    , m_top_box(0.175, 0.715, 0.649, 0.005)
    , m_bottom_box(0.177, 0.896, 0.645, 0.008)
    , m_selection_box(0.783, 0.457, 0.017, 0.177)
{}
void SelectionDialogDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_right_box);
    items.add(COLOR_RED, m_top_box);
    items.add(COLOR_RED, m_bottom_box);
    items.add(COLOR_RED, m_selection_box);
}
bool SelectionDialogDetector::detect(const ImageViewRGB32& screen){
    ImageViewRGB32 right_image = extract_box_reference(screen, m_right_box);
    ImageViewRGB32 top_image = extract_box_reference(screen, m_top_box);
    ImageViewRGB32 bottom_image = extract_box_reference(screen, m_bottom_box);
    ImageViewRGB32 selection_image = extract_box_reference(screen, m_selection_box);
    if (is_solid(right_image, { 0.25, 0.38, 0.369 })
        && is_solid(top_image, { 0.25, 0.38, 0.369 })
        && is_solid(bottom_image, { 0.25, 0.38, 0.369 })
        && is_solid(selection_image, { 0.25, 0.38, 0.369 })
        ){
        return true;
    }
    return false;
}

BattleDialogDetector::BattleDialogDetector(Color color)
    : m_dialog_top_box(0.152, 0.721, 0.694, 0.008)
    , m_dialog_right_box(0.842, 0.730, 0.004, 0.171)
{}
void BattleDialogDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_dialog_top_box);
    items.add(COLOR_RED, m_dialog_right_box);
}
bool BattleDialogDetector::detect(const ImageViewRGB32& screen){
    //Background dialog is teal
    ImageViewRGB32 dialog_top_image = extract_box_reference(screen, m_dialog_top_box);
    ImageViewRGB32 dialog_right_image = extract_box_reference(screen, m_dialog_right_box);

    if (is_solid(dialog_top_image, { 0.176, 0.357, 0.467 })
        && is_solid(dialog_right_image, { 0.176, 0.357, 0.467 })
        ){
        return true;
    }
    return false;
}


BattleMenuDetector::BattleMenuDetector(Color color)
    : m_menu_top_box(0.522, 0.716, 0.331, 0.011)
    , m_menu_right_box(0.845, 0.727, 0.008, 0.175)
    , m_dialog_top_box(0.153, 0.720, 0.343, 0.014) //top of the white dialog box
    , m_dialog_right_box(0.486, 0.722, 0.009, 0.177) //right side, closest to the menu
{}
void BattleMenuDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_menu_top_box);
    items.add(COLOR_RED, m_menu_right_box);
    items.add(COLOR_RED, m_dialog_top_box);
    items.add(COLOR_RED, m_dialog_right_box);
}
bool BattleMenuDetector::detect(const ImageViewRGB32& screen){
    //Menu is white
    ImageViewRGB32 menu_top_image = extract_box_reference(screen, m_menu_top_box);
    ImageViewRGB32 menu_right_image = extract_box_reference(screen, m_menu_right_box);

    //Background dialog is teal
    ImageViewRGB32 dialog_top_image = extract_box_reference(screen, m_dialog_top_box);
    ImageViewRGB32 dialog_right_image = extract_box_reference(screen, m_dialog_right_box);

    if (is_solid(menu_top_image, { 0.335, 0.332, 0.335 }) //253, 251, 254 white
        && is_solid(menu_right_image, { 0.335, 0.332, 0.335 })
        && is_solid(dialog_top_image, { 0.176, 0.357, 0.467 }) //40, 81, 106 teal
        && is_solid(dialog_right_image, { 0.176, 0.357, 0.467 })
        ){
        return true;
    }
    return false;
}


AdvanceBattleDialogDetector::AdvanceBattleDialogDetector(Color color)
    : m_dialog_box(0.152, 0.721, 0.694, 0.177)
    , m_dialog_top_box(0.152, 0.721, 0.694, 0.008)
    , m_dialog_right_box(0.842, 0.730, 0.004, 0.171)
{}
void AdvanceBattleDialogDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_dialog_box);
    items.add(COLOR_RED, m_dialog_top_box);
    items.add(COLOR_RED, m_dialog_right_box);
}
bool AdvanceBattleDialogDetector::detect(const ImageViewRGB32& screen){
    const bool replace_color_within_range = false;

    //Filter out background
    ImageRGB32 filtered_region = filter_rgb32_range(
        extract_box_reference(screen, m_dialog_box),
        combine_rgb(185, 0, 1), combine_rgb(255, 32, 33), Color(0), replace_color_within_range
    );
    ImageStats stats = image_stats(filtered_region);

    /*
    filtered_region.save("./filtered_only.png");
    cout << stats.average.r << endl;
    cout << stats.average.g << endl;
    cout << stats.average.b << endl;
    */

    ImageViewRGB32 dialog_top_image = extract_box_reference(screen, m_dialog_top_box);
    ImageViewRGB32 dialog_right_image = extract_box_reference(screen, m_dialog_right_box);

    if (is_solid(dialog_top_image, { 0.176, 0.357, 0.467 })
        && is_solid(dialog_right_image, { 0.176, 0.357, 0.467 })
        && (stats.average.r > stats.average.b + 200)
        && (stats.average.r > stats.average.g + 200)
        )
    {
        return true;
    }
    return false;
}


}
}
}
