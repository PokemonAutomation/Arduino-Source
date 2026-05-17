/*  Daycare Man Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "PokemonFRLG/PokemonFRLG_Settings.h"
#include "PokemonFRLG_DaycareManDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

DaycareManDetector::DaycareManDetector(Color color)
    : m_box_man(0.741827, 0.492067, 0.052404, 0.009375)     // rgb(98, 219, 173) 
    , m_box_grass(0.345047, 0.798244, 0.079690, 0.056954)   // rgb(98, 219, 173)     
    , m_box_puddle(0.892758, 0.025567, 0.059451, 0.091126)  // rgb(82, 177, 255)
    , m_box_sand(0.103447, 0.513473, 0.026563, 0.030375)    // rgb(255, 251, 127)
    , m_box_roof(0.744762, 0.767868, 0.007590, 0.127197)    // rgb(255, 93, 60)  
{}
void DaycareManDetector::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_box_grass));
    items.add(COLOR_GREEN, GAME_BOX.inner_to_outer(m_box_grass));
    items.add(COLOR_BLUE, GAME_BOX.inner_to_outer(m_box_puddle));
    items.add(COLOR_YELLOW, GAME_BOX.inner_to_outer(m_box_sand));
    items.add(COLOR_ORANGE, GAME_BOX.inner_to_outer(m_box_roof));
}
bool DaycareManDetector::detect(const ImageViewRGB32& screen){
    ImageViewRGB32 game_screen = extract_box_reference(screen, GameSettings::instance().GAME_BOX);

    return (
          !is_solid(extract_box_reference(game_screen, m_box_man),    { 0.200, 0.447, 0.353 }, 0.25, 20) // if egg is ready, the man is blocking the grass
        && is_solid(extract_box_reference(game_screen, m_box_grass),  { 0.200, 0.447, 0.353 }, 0.25, 20)
        && is_solid(extract_box_reference(game_screen, m_box_puddle), { 0.160, 0.344, 0.496 }, 0.25, 20)
        && is_solid(extract_box_reference(game_screen, m_box_sand),   { 0.403, 0.396, 0.201 }, 0.25, 20)
        && is_solid(extract_box_reference(game_screen, m_box_roof),   { 0.625, 0.228, 0.147 }, 0.25, 20)
    );
}


}
}
}

