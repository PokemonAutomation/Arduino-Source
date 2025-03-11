/*  Battle Sprite Watcher
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <vector>
#include "Common/Cpp/AbstractLogger.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/Tools/DebugDumper.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "PokemonLA_BattleSpriteWatcher.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

const size_t MAX_WILD_POKEMON_IN_MULTI_BATTLE = 8;



BattleSpriteWatcher::BattleSpriteWatcher(Logger& logger, VideoOverlay& overlay)
    : VisualInferenceCallback("BattleSpriteWatcher")
    , m_logger(logger)
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

    const bool found_new_sprites = set_detected_sprites(frame, m_sprite_appeared);
    if (found_new_sprites && PreloadSettings::instance().DEVELOPER_MODE){
        dump_debug_image(m_logger, "PokemonLA/BattleSpriteWatcher", "SpriteDetected", frame);
    }

    return false;
}


std::vector<bool> BattleSpriteWatcher::detect_sprites(const ImageViewRGB32& frame) const{
    std::vector<bool> ret(m_sprite_boxes.size(), false);
    set_detected_sprites(frame, ret);
    return ret;
}


bool BattleSpriteWatcher::set_detected_sprites(const ImageViewRGB32& frame, std::vector<bool>& sprites) const{
    bool ret = false;
    for(size_t i = 0; i < m_sprite_boxes.size(); i++){
        const auto& box = m_sprite_boxes[i];
        const auto stats = image_stats(extract_box_reference(frame, box));
        // cout << stats.average.to_string() << " " << stats.stddev.to_string() << endl;
        if (sprites[i] == false && (stats.average.sum() > 150.0 || stats.stddev.sum() > 60.0)){
            ret = true;
            sprites[i] = true;
        }
    }

    return ret;
}


}
}
}
