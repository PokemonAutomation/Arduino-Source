/*  Battle Sprite Watcher
 *
 *  From: https://github.com/PokemonAutomation/
 *  
 *  Watch the sprites appear on the upper right corner of the screen during a mult-pokemon
 *  battle. This is useful to detect if any skittish pokemon escape during the starting
 *  stage of the battle.
 */

#ifndef PokemonAutomation_PokemonLA_BattleSpriteWatcher_H
#define PokemonAutomation_PokemonLA_BattleSpriteWatcher_H

#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "PokemonLA/Inference/Battles/PokemonLA_BattleStartDetector.h"

namespace PokemonAutomation{

class Logger;

namespace NintendoSwitch{
namespace PokemonLA{

// Max number of wild pokemon that can enter a multi battle against player
extern const size_t MAX_WILD_POKEMON_IN_MULTI_BATTLE;

class BattleSpriteWatcher : public VisualInferenceCallback{
public:
    BattleSpriteWatcher(Logger& logger, VideoOverlay& overlay);

    virtual void make_overlays(VideoOverlaySet& items) const override;

    // As a watcher, this function always returns false.
    // If it detects a sprite appears, it sets the corresponding bool in `m_sprite_appeared` to true.
    // After inference session ends, call `sprites_appeared() to access it.
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

    // Read whether a sprite appeared duing an inference session, where BattleSpriteWatcher::process_frame() is called.
    // Return a vector of `MAX_WILD_POKEMON_IN_MULTI_BATTLE` bools.
    // The sprites are indexed from right to left, with the rightmost sprite with index 0.
    const std::vector<bool>& sprites_appeared() const { return m_sprite_appeared; }

    // Detect whether a sprite is present on the input frame.
    // Does not modify any internal data of `BattleSpriteWatcher`.
    // Return a vector of `MAX_WILD_POKEMON_IN_MULTI_BATTLE` bools.
    // The sprites are indexed from right to left, with the rightmost sprite with index 0.
    std::vector<bool> detect_sprites(const ImageViewRGB32& frame) const;

private:

    // Set the bool value of the detected sprites in `sprites` to true
    // Return true if at least one sprite is newly detected
    bool set_detected_sprites(const ImageViewRGB32& frame, std::vector<bool>& sprites) const;

    Logger& m_logger;

    BattleStartDetector m_battle_start_detector;
    std::vector<ImageFloatBox> m_sprite_boxes;
    std::vector<bool> m_sprite_appeared;
};


}
}
}
#endif
