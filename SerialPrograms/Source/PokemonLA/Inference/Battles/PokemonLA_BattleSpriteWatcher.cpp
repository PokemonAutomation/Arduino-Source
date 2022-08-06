/*  Battle Sprite Watcher
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/AbstractLogger.h"
#include "CommonFramework/ImageTools/ImageGradient.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "CommonFramework/Logging/LoggerQt.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "PokemonLA_BattleSpriteWatcher.h"

#include <vector>
#include <iostream>
#include <algorithm>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

const size_t MAX_WILD_POKEMON_IN_MULTI_BATTLE = 8;



BattleSpriteWatcher::BattleSpriteWatcher(Logger& logger, VideoOverlay& overlay)
    : VisualInferenceCallback("BattleSpriteWatcher")
    , m_battle_start_detector(logger, overlay)
{
    for(size_t i = 0; i < MAX_WILD_POKEMON_IN_MULTI_BATTLE; i++){
        m_sprite_boxes.emplace_back(0.957 - 0.035*i, 0.044, 0.021, 0.035);
    }
    m_sprite_appeared.resize(m_sprite_boxes.size(), false);
}

void BattleSpriteWatcher::make_overlays(VideoOverlaySet& items) const{
    for(const auto& box : m_sprite_boxes){
        items.add(COLOR_RED, box);
    }
}

bool BattleSpriteWatcher::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    bool in_battle = m_battle_start_detector.process_frame(frame, timestamp);
    if (in_battle == false){
        return false;
    }

    set_detected_sprites(frame, m_sprite_appeared);

    return false;
}


std::vector<bool> BattleSpriteWatcher::detect_sprites(const ImageViewRGB32& frame) const{
    std::vector<bool> ret(m_sprite_boxes.size(), false);
    set_detected_sprites(frame, ret);
    return ret;
}


void BattleSpriteWatcher::set_detected_sprites(const ImageViewRGB32& frame, std::vector<bool>& sprites) const{
    for(size_t i = 0; i < m_sprite_boxes.size(); i++){
        const auto& box = m_sprite_boxes[i];
        const auto stats = image_stats(extract_box_reference(frame, box));
        // cout << stats.average.to_string() << " " << stats.stddev.to_string() << endl;
        if (stats.average.sum() > 150.0 || stats.stddev.sum() > 20.0){
            sprites[i] = true;
        }
    }
}


}
}
}
