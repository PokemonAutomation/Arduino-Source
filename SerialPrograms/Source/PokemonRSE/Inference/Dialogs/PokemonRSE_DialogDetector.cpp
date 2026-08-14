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


DialogDetector::DialogDetector(Color color)
    : m_top_box(0.13, 0.745, 0.7, 0.005)
    , m_inner_box(0.044217, 0.756643, 0.007092, 0.17875)

    , m_side_box_jpn(0.067267, 0.763312, 0.018618, 0.170746)
    , m_inner_box_jpn(0.114254, 0.771316, 0.007092, 0.161409) //white
{}
void DialogDetector::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_top_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_inner_box));

    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_side_box_jpn));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_inner_box_jpn));
}
bool DialogDetector::detect(const ImageViewRGB32& screen){
    ImageViewRGB32 game_screen = extract_box_reference(screen, GameSettings::instance().GAME_BOX);

    ImageViewRGB32 dialog_top_image = extract_box_reference(game_screen, m_top_box);
    ImageViewRGB32 dialog_inner_image = extract_box_reference(game_screen, m_inner_box);

    ImageViewRGB32 dialog_side_image_jpn = extract_box_reference(game_screen, m_side_box_jpn);
    ImageViewRGB32 dialog_inner_image_jpn = extract_box_reference(game_screen, m_inner_box_jpn);

    if ((is_white(dialog_inner_image)
        && is_white(dialog_top_image))
        ||
        (is_white(dialog_inner_image_jpn)
        && (is_solid(dialog_side_image_jpn, { 0.2300556, 0.34508, 0.42486 }, 0.25, 20) //RS - blue - 124 186 229
            || is_solid(dialog_side_image_jpn, { 0.0025575, 0.516624, 0.4808184 }, 0.25, 20))) //E - green - 1 202 188
    ){
        return true;
    }
    return false;
}



AdvanceDialogDetector::AdvanceDialogDetector(Color color)
    : m_top_box(0.13, 0.745, 0.7, 0.005)
    , m_inner_box(0.044217, 0.756643, 0.007092, 0.17875)
    , m_dialog_box(0.041557, 0.748639, 0.918467, 0.205429)

    , m_side_box_jpn(0.067267, 0.763312, 0.018618, 0.170746)
    , m_inner_box_jpn(0.114254, 0.771316, 0.007092, 0.161409)
    , m_dialog_jpn_box(0.116914, 0.747305, 0.765094, 0.209431)
{}
void AdvanceDialogDetector::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_top_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_inner_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_dialog_box));

    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_side_box_jpn));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_inner_box_jpn));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_dialog_jpn_box));
}
bool AdvanceDialogDetector::detect(const ImageViewRGB32& screen){
    ImageViewRGB32 game_screen = extract_box_reference(screen, GameSettings::instance().GAME_BOX);

    ImageViewRGB32 dialog_top_image = extract_box_reference(game_screen, m_top_box);
    ImageViewRGB32 dialog_inner_image = extract_box_reference(game_screen, m_inner_box);

    ImageViewRGB32 dialog_side_image_jpn = extract_box_reference(game_screen, m_side_box_jpn);
    ImageViewRGB32 dialog_inner_image_jpn = extract_box_reference(game_screen, m_inner_box_jpn);

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

    if ((is_white(dialog_inner_image)
        && is_white(dialog_top_image)
        && (stats.average.r > stats.average.b + 180)
        && (stats.average.r > stats.average.g + 180))
        ||
        (is_white(dialog_inner_image_jpn)
        && (is_solid(dialog_side_image_jpn, { 0.2300556, 0.34508, 0.42486 }, 0.25, 20) //RS - blue - 124 186 229
            || is_solid(dialog_side_image_jpn, { 0.0025575, 0.516624, 0.4808184 }, 0.25, 20)) //E - green - 1 202 188
        && (stats2.average.r > stats2.average.b + 180)
        && (stats2.average.r > stats2.average.g + 180))
    ){
        return true;
    }
    return false;
}



SelectionDialogDetector::SelectionDialogDetector(Color color)
    : m_top_box(0.13, 0.745, 0.7, 0.005)
    , m_inner_box(0.044217, 0.756643, 0.007092, 0.17875)

    , m_side_box_jpn(0.067267, 0.763312, 0.018618, 0.170746)
    , m_inner_box_jpn(0.114254, 0.771316, 0.007092, 0.161409) //white

    , m_yes_box(0.85, 0.46, 0.007, 0.069)
    , m_no_box(0.85, 0.57, 0.007, 0.069)
{}
void SelectionDialogDetector::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_top_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_inner_box));

    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_side_box_jpn));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_inner_box_jpn));

    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_yes_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_no_box));
}
bool SelectionDialogDetector::detect(const ImageViewRGB32& screen){
    ImageViewRGB32 game_screen = extract_box_reference(screen, GameSettings::instance().GAME_BOX);

    ImageViewRGB32 dialog_top_image = extract_box_reference(game_screen, m_top_box);
    ImageViewRGB32 dialog_inner_image = extract_box_reference(game_screen, m_inner_box);

    ImageViewRGB32 dialog_side_image_jpn = extract_box_reference(game_screen, m_side_box_jpn);
    ImageViewRGB32 dialog_inner_image_jpn = extract_box_reference(game_screen, m_inner_box_jpn);

    ImageViewRGB32 dialog_yes_image = extract_box_reference(game_screen, m_yes_box);
    ImageViewRGB32 dialog_no_image = extract_box_reference(game_screen, m_no_box);

    if ((is_white(dialog_yes_image) && is_white(dialog_no_image))
        && ((is_white(dialog_inner_image) && is_white(dialog_top_image))
            ||
            (is_white(dialog_inner_image_jpn)
            && (is_solid(dialog_side_image_jpn, { 0.2300556, 0.34508, 0.42486 }, 0.25, 20) //RS - blue - 124 186 229
                || is_solid(dialog_side_image_jpn, { 0.0025575, 0.516624, 0.4808184 }, 0.25, 20)))) //E - green - 1 202 188
    ){
        return true;
    }
    return false;
}



}
}
}
