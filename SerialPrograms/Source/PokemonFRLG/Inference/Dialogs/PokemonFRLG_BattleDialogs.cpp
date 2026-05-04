/*  Battle Dialog Detectors
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonTools/Images/SolidColorTest.h"
#include "CommonTools/Images/ImageFilter.h"
#include "PokemonFRLG/PokemonFRLG_Settings.h"
#include "PokemonFRLG_BattleDialogs.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{



BattleDialogDetector::BattleDialogDetector(Color color)
    : m_dialog_top_box(0.0372308, 0.750154, 0.925538, 0.00934615)
    , m_dialog_right_box(0.956615, 0.7595, 0.00615385, 0.185885)
    , m_dialog_top_jpn_box(0.043890, 0.749227, 0.910248, 0.008023) //jpn positions might work for other languages
    , m_dialog_right_jpn_box(0.950583, 0.751901, 0.003556, 0.199230) //dialog might get in the way though
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


BattleMenuDetector::BattleMenuDetector(Color color)
    : m_menu_top_box(0.528308, 0.742885, 0.439385, 0.00830769) //top of the white dialog box
    , m_menu_right_box(0.961538, 0.752231, 0.00615385, 0.200423)
    , m_dialog_top_box(0.036, 0.749115, 0.459077, 0.0135)
    , m_dialog_right_box(0.490154, 0.762615, 0.00615385, 0.178615) //right side, closest to the menu
    , m_menu_top_jpn_box(0.593239, 0.743878, 0.373344, 0.009360) //very different positions!
    , m_menu_right_jpn_box(0.961538, 0.752231, 0.00615385, 0.200423)
    , m_dialog_top_jpn_box(0.043001, 0.750564, 0.520015, 0.002674)
    , m_dialog_right_jpn_box(0.559460, 0.750564, 0.003556, 0.196556)
{}
void BattleMenuDetector::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_menu_top_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_menu_right_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_dialog_top_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_dialog_right_box));

    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_menu_top_jpn_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_menu_right_jpn_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_dialog_top_jpn_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_dialog_right_jpn_box));
}
bool BattleMenuDetector::detect(const ImageViewRGB32& screen){
    ImageViewRGB32 game_screen = extract_box_reference(screen, GameSettings::instance().GAME_BOX);

    //Menu is white
    ImageViewRGB32 menu_top_image = extract_box_reference(game_screen, m_menu_top_box);
    ImageViewRGB32 menu_right_image = extract_box_reference(game_screen, m_menu_right_box);
    ImageViewRGB32 menu_top_jpn_image = extract_box_reference(game_screen, m_menu_top_jpn_box);
    ImageViewRGB32 menu_right_jpn_image = extract_box_reference(game_screen, m_menu_right_jpn_box);

    //Background dialog is teal
    ImageViewRGB32 dialog_top_image = extract_box_reference(game_screen, m_dialog_top_box);
    ImageViewRGB32 dialog_right_image = extract_box_reference(game_screen, m_dialog_right_box);
    ImageViewRGB32 dialog_top_jpn_image = extract_box_reference(game_screen, m_dialog_top_jpn_box);
    ImageViewRGB32 dialog_right_jpn_image = extract_box_reference(game_screen, m_dialog_right_jpn_box);

    if ((is_white(menu_top_image) //All languages except japanese
        && is_white(menu_right_image)
        && is_solid(dialog_top_image, { 0.176, 0.357, 0.467 }, 0.25, 20) //40, 81, 106 teal
        && is_solid(dialog_right_image, { 0.176, 0.357, 0.467 }, 0.25, 20)
        && !is_white(dialog_top_image))
        ||
        (is_white(menu_top_jpn_image) //japanese
        && is_white(menu_right_jpn_image)
        && is_solid(dialog_top_jpn_image, { 0.176, 0.357, 0.467 }, 0.25, 20)
        && is_solid(dialog_right_jpn_image, { 0.176, 0.357, 0.467 }, 0.25, 20)
        && !is_white(dialog_top_jpn_image))
    ){
        return true;
    }
    return false;
}


AdvanceBattleDialogDetector::AdvanceBattleDialogDetector(Color color)
    : m_dialog_box(0.036, 0.748077, 0.926769, 0.204577)
    , m_dialog_top_box(0.0372308, 0.750154, 0.925538, 0.00934615)
    , m_dialog_right_box(0.956615, 0.7595, 0.00615385, 0.185885)
    , m_dialog_jpn_box(0.043890, 0.749227, 0.911137, 0.203242) //this position is very different!
    , m_dialog_top_jpn_box(0.043890, 0.749227, 0.910248, 0.008023)
    , m_dialog_right_jpn_box(0.950583, 0.751901, 0.003556, 0.199230)
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

BattleOutOfPpDetector::BattleOutOfPpDetector(Color color)
    : m_box(0.692901, 0.758376, 0.275753, 0.085431)
    , m_area_ratio_threshold(0.01)
{}
void BattleOutOfPpDetector::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_box));
}
bool BattleOutOfPpDetector::detect(const ImageViewRGB32& screen){
    ImageViewRGB32 game_screen = extract_box_reference(screen, GameSettings::instance().GAME_BOX);
    const auto region = extract_box_reference(game_screen, m_box);

    // Retain only red pixels from region ( ~ RGB(225, 74, 27) )
    const bool replace_color_within_range = false;
    const ImageRGB32 red_region = filter_rgb32_range(
        region,
        combine_rgb(180, 0, 0), combine_rgb(255, 120, 120), Color(0), replace_color_within_range
    );
    const size_t num_red_pixels = image_stats(red_region).count;
    const double threshold = region.width() * region.height() * m_area_ratio_threshold;

    return num_red_pixels > threshold;
}

BattleLevelUpDetector::BattleLevelUpDetector(Color color, BattleLevelUpDialog dialog_type)
    : dialog_type(dialog_type)
    , m_border_top_box(0.619231, 0.374038, 0.362179, 0.001923) // gray (120, 115, 140)
    , m_border_right_box(0.982692, 0.376923, 0.001923, 0.597115)
    , m_dialog_top_box(0.626282, 0.393492, 0.341026, 0.006175)  // white
    , m_dialog_right_box(0.967949, 0.401923, 0.003846, 0.550962)
    , m_plus_box(0.862663, 0.402852, 0.034267, 0.553560)
{}
void BattleLevelUpDetector::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_border_top_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_border_right_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_dialog_top_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_dialog_right_box));
}
bool BattleLevelUpDetector::detect(const ImageViewRGB32& screen){
    ImageViewRGB32 game_screen = extract_box_reference(screen, GameSettings::instance().GAME_BOX);

    //Border is teal
    ImageViewRGB32 border_top_image = extract_box_reference(game_screen, m_border_top_box);
    ImageViewRGB32 border_right_image = extract_box_reference(game_screen, m_border_right_box);

    //Menu is white
    ImageViewRGB32 dialog_top_image = extract_box_reference(game_screen, m_dialog_top_box);
    ImageViewRGB32 dialog_right_image = extract_box_reference(game_screen, m_dialog_right_box);
   
    //Plus box is not solid white on the first screen, but is white on the second one
    ImageViewRGB32 plus_image = extract_box_reference(game_screen, m_plus_box);
    bool good_plus_image = (
        dialog_type == BattleLevelUpDialog::either
        || (dialog_type == BattleLevelUpDialog::plus && !is_white(plus_image))
        || (dialog_type == BattleLevelUpDialog::stats && is_white(plus_image))
    );

    if (is_solid(border_top_image, { 0.320, 0.307, 0.373 }, 0.25, 20)
        && is_solid(border_right_image, { 0.320, 0.307, 0.373 }, 0.25, 20)
        && is_white(dialog_top_image)
        && is_white(dialog_right_image)
        && good_plus_image
    ){
        return true;
    }
    return false;
}



}
}
}
