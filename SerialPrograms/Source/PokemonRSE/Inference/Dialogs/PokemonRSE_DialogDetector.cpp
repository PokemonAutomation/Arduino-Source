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
#include "PokemonRSE_DialogDetector.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonRSE{

/*
DialogDetector::DialogDetector(Color color)
    : m_left_box(0.155, 0.727, 0.015, 0.168)
    , m_right_box(0.837, 0.729, 0.008, 0.161)
{}
void DialogDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_left_box);
    items.add(COLOR_RED, m_right_box);
}
bool DialogDetector::detect(const ImageViewRGB32& screen) const{
    ImageViewRGB32 left_image = extract_box_reference(screen, m_left_box);
    ImageViewRGB32 right_image = extract_box_reference(screen, m_right_box);
    if (is_solid(left_image, { 0.335, 0.331, 0.332 }) && is_solid(right_image, { 0.335, 0.331, 0.332 })){
        return true;
    }
    return false;
}
*/

BattleDialogDetector::BattleDialogDetector(Color color)
    : m_left_box(0.158, 0.725, 0.011, 0.176)
    , m_right_box(0.827, 0.722, 0.013, 0.178)
{}
void BattleDialogDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_left_box);
    items.add(COLOR_RED, m_right_box);
}
bool BattleDialogDetector::detect(const ImageViewRGB32& screen){
    ImageViewRGB32 left_image = extract_box_reference(screen, m_left_box);
    ImageViewRGB32 right_image = extract_box_reference(screen, m_right_box);
    if (is_solid(left_image, { 0.335, 0.331, 0.332 }) && is_solid(right_image, { 0.335, 0.331, 0.332 })){
        return true;
    }
    return false;
}


BattleMenuDetector::BattleMenuDetector(Color color)
    : m_left_box(0.439, 0.717, 0.021, 0.192)
    , m_right_box(0.821, 0.725, 0.030, 0.181)
{}
void BattleMenuDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_left_box);
    items.add(COLOR_RED, m_right_box);
}
bool BattleMenuDetector::detect(const ImageViewRGB32& screen){
    ImageViewRGB32 left_image = extract_box_reference(screen, m_left_box);
    ImageViewRGB32 right_image = extract_box_reference(screen, m_right_box);
    if (is_solid(left_image, { 0.335, 0.331, 0.332 }) && is_solid(right_image, { 0.25, 0.38, 0.369 })){
        return true;
    }
    return false;
}


AdvanceBattleDialogDetector::AdvanceBattleDialogDetector(Color color)
    : m_dialog_box(0.156, 0.715, 0.686, 0.193)
    , m_left_box(0.156, 0.724, 0.010, 0.176)
    , m_right_box(0.836, 0.717, 0.008, 0.189)
{}
void AdvanceBattleDialogDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_dialog_box);
    items.add(COLOR_RED, m_left_box);
    items.add(COLOR_RED, m_right_box);
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

    ImageViewRGB32 left_image = extract_box_reference(screen, m_left_box);
    ImageViewRGB32 right_image = extract_box_reference(screen, m_right_box);

    if (is_solid(left_image, { 0.335, 0.331, 0.332 })
        && is_solid(right_image, { 0.335, 0.331, 0.332 })
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
