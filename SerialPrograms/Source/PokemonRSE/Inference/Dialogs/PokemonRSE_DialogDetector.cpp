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

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonRSE{

/*
BattleDialogDetector::BattleDialogDetector(Color color)
    : m_dialog_top_box(0.049224, 0.738530, 0.901359, 0.008023)
    , m_dialog_right_box(0.943471, 0.746553, 0.007111, 0.212602)
    , m_dialog_top_jpn_box(0.068780, 0.738530, 0.856913, 0.012034)
    , m_dialog_right_jpn_box(0.918582, 0.747890, 0.007111, 0.208590)
{}
void BattleDialogDetector::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_dialog_top_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_dialog_right_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_dialog_top_jpn_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_dialog_right_jpn_box));
}
bool BattleDialogDetector::detect(const ImageViewRGB32& screen){
    ImageViewRGB32 game_screen = extract_box_reference(screen, GameSettings::instance().GAME_BOX);

    ImageViewRGB32 dialog_top_image = extract_box_reference(game_screen, m_dialog_top_box);
    ImageViewRGB32 dialog_right_image = extract_box_reference(game_screen, m_dialog_right_box);
    ImageViewRGB32 dialog_top_jpn_image = extract_box_reference(game_screen, m_dialog_top_jpn_box);
    ImageViewRGB32 dialog_right_jpn_image = extract_box_reference(game_screen, m_dialog_right_jpn_box);

    if ((is_solid(dialog_top_image, { 0.176, 0.357, 0.467 }, 0.25, 20)
        && is_solid(dialog_right_image, { 0.176, 0.357, 0.467 }, 0.25, 20))
        ||
        (is_solid(dialog_top_jpn_image, { 0.176, 0.357, 0.467 }, 0.25, 20)
        && is_solid(dialog_right_jpn_image, { 0.176, 0.357, 0.467 }, 0.25, 20))
    ){
        return true;
    }
    return false;
}
*/

BattleMenuDetector::BattleMenuDetector(Color color)
    : m_menu_top_box(0.599462, 0.738530, 0.366233, 0.004011) //top of the white box
    , m_menu_bottom_box(0.602128, 0.947120, 0.368010, 0.006686)
    , m_dialog_top_box(0.068780, 0.739867, 0.488903, 0.008023) //teal boxes
    , m_dialog_right_box(0.553238, 0.739867, 0.006222, 0.215276)

    , m_menu_top_eme_box(0.534571, 0.746553, 0.434679, 0.008023)
    , m_menu_right_eme_box(0.962138, 0.747890, 0.007111, 0.205916)
    , m_dialog_top_eme_box(0.046557, 0.741204, 0.442679, 0.008023)
    , m_dialog_right_eme_box(0.487458, 0.742541, 0.004445, 0.216613)
{}
void BattleMenuDetector::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_menu_top_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_menu_bottom_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_dialog_top_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_dialog_right_box));

    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_menu_top_eme_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_menu_right_eme_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_dialog_top_eme_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_dialog_right_eme_box));
}
bool BattleMenuDetector::detect(const ImageViewRGB32& screen){
    ImageViewRGB32 game_screen = extract_box_reference(screen, GameSettings::instance().GAME_BOX);

    //Menu is white
    ImageViewRGB32 menu_top_image = extract_box_reference(game_screen, m_menu_top_box);
    ImageViewRGB32 menu_bottom_image = extract_box_reference(game_screen, m_menu_bottom_box);
    ImageViewRGB32 menu_top_jpn_image = extract_box_reference(game_screen, m_menu_top_eme_box);
    ImageViewRGB32 menu_right_jpn_image = extract_box_reference(game_screen, m_menu_right_eme_box);

    //Background dialog is teal
    ImageViewRGB32 dialog_top_image = extract_box_reference(game_screen, m_dialog_top_box);
    ImageViewRGB32 dialog_right_image = extract_box_reference(game_screen, m_dialog_right_box);
    ImageViewRGB32 dialog_top_jpn_image = extract_box_reference(game_screen, m_dialog_top_eme_box);
    ImageViewRGB32 dialog_right_jpn_image = extract_box_reference(game_screen, m_dialog_right_eme_box);

    if ((is_white(menu_top_image) //Ruby/Sapphire, all languages. Emerald Japan.
        && is_white(menu_bottom_image)
        && is_solid(dialog_top_image, { 0.176, 0.357, 0.467 }, 0.25, 20)
        && is_solid(dialog_right_image, { 0.176, 0.357, 0.467 }, 0.25, 20))
        ||
        (is_white(menu_top_jpn_image) //Emerald, all languages except Japan.
        && is_white(menu_right_jpn_image)
        && is_solid(dialog_top_jpn_image, { 0.176, 0.357, 0.467 }, 0.25, 20)
        && is_solid(dialog_right_jpn_image, { 0.176, 0.357, 0.467 }, 0.25, 20))
    ){
        return true;
    }
    return false;
}


AdvanceBattleDialogDetector::AdvanceBattleDialogDetector(Color color)
    : m_dialog_box(0.043890, 0.737193, 0.913804, 0.227310)
    , m_dialog_top_box(0.049224, 0.738530, 0.901359, 0.008023)
    , m_dialog_right_box(0.943471, 0.746553, 0.007111, 0.212602)
    , m_dialog_jpn_box(0.059891, 0.742541, 0.872914, 0.216613)
    , m_dialog_top_jpn_box(0.068780, 0.738530, 0.856913, 0.012034)
    , m_dialog_right_jpn_box(0.918582, 0.747890, 0.007111, 0.208590)
{}
void AdvanceBattleDialogDetector::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_dialog_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_dialog_top_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_dialog_right_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_dialog_jpn_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_dialog_top_jpn_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_dialog_right_jpn_box));
}
bool AdvanceBattleDialogDetector::detect(const ImageViewRGB32& screen){
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

    //japanese
    ImageRGB32 filtered_region_jpn = filter_rgb32_range(
        extract_box_reference(game_screen, m_dialog_jpn_box),
        combine_rgb(164, 0, 0), combine_rgb(255, 114, 87), Color(0), replace_color_within_range
    );
    ImageStats stats2 = image_stats(filtered_region_jpn);

    ImageViewRGB32 dialog_top_image = extract_box_reference(game_screen, m_dialog_top_box);
    ImageViewRGB32 dialog_right_image = extract_box_reference(game_screen, m_dialog_right_box);
    ImageViewRGB32 dialog_top_jpn_image = extract_box_reference(game_screen, m_dialog_top_jpn_box);
    ImageViewRGB32 dialog_right_jpn_image = extract_box_reference(game_screen, m_dialog_right_jpn_box);

    if ((is_solid(dialog_top_image, { 0.176, 0.357, 0.467 }, 0.25, 20)
        && is_solid(dialog_right_image, { 0.176, 0.357, 0.467 }, 0.25, 20)
        && (stats.average.r > stats.average.b + 180)
        && (stats.average.r > stats.average.g + 180))
        ||
        (is_solid(dialog_top_jpn_image, { 0.176, 0.357, 0.467 }, 0.25, 20)
        && is_solid(dialog_right_jpn_image, { 0.176, 0.357, 0.467 }, 0.25, 20)
        && (stats2.average.r > stats2.average.b + 180)
        && (stats2.average.r > stats2.average.g + 180))
    ){
        return true;
    }
    return false;
}



}
}
}
