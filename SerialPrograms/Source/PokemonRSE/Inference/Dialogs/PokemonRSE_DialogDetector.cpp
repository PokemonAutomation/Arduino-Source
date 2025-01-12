/*  Dialog Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "PokemonRSE_DialogDetector.h"

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


BattleDialogDetector::BattleDialogDetector(Color color)
    : m_left_box(0.155, 0.727, 0.015, 0.168)
    , m_right_box(0.837, 0.729, 0.008, 0.161)
{}
void BattleDialogDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_left_box);
    items.add(COLOR_RED, m_right_box);
}
bool BattleDialogDetector::detect(const ImageViewRGB32& screen) const{
    ImageViewRGB32 left_image = extract_box_reference(screen, m_left_box);
    ImageViewRGB32 right_image = extract_box_reference(screen, m_right_box);
    if (is_solid(left_image, { 0.25, 0.38, 0.369 }) && is_solid(right_image, { 0.25, 0.38, 0.369 })){
        return true;
    }
    return false;
}
*/

BattleMenuDetector::BattleMenuDetector(Color color)
    : m_left_box(0.155, 0.727, 0.015, 0.168)
    , m_right_box(0.821, 0.725, 0.030, 0.181)
{}
void BattleMenuDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_left_box);
    items.add(COLOR_RED, m_right_box);
}
bool BattleMenuDetector::detect(const ImageViewRGB32& screen) const{
    ImageViewRGB32 left_image = extract_box_reference(screen, m_left_box);
    ImageViewRGB32 right_image = extract_box_reference(screen, m_right_box);
    if (is_solid(left_image, { 0.335, 0.331, 0.332 }) && is_solid(right_image, { 0.25, 0.38, 0.369 })){
        return true;
    }
    return false;
}



}
}
}
