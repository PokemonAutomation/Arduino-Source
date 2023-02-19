/*  Let's Go HP Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "Pokemon/Inference/Pokemon_ReadHpBar.h"
#include "PokemonSV_LetsGoHpReader.h"

namespace PokemonAutomation{
    class Logger;
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;



LetsGoHpWatcher::LetsGoHpWatcher(Color color)
    : VisualInferenceCallback("LetsGoHpWatcher")
    , m_color(color)
    , m_box(0.055, 0.928, 0.067, 0.012)
    , m_last_known_value(-1)
{}

void LetsGoHpWatcher::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}
bool LetsGoHpWatcher::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    double hp = read_hp_bar(
        extract_box_reference(frame, ImageFloatBox(0.055, 0.928, 0.067, 0.012))
    );
    if (0 < hp){
        m_last_known_value.store(hp, std::memory_order_relaxed);
    }
    return false;
}




}
}
}
