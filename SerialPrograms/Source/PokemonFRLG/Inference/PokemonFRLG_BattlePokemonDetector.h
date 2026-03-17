/*  Map Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_BattlePokemonDetector_H
#define PokemonAutomation_PokemonFRLG_BattlePokemonDetector_H

#include <chrono>
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/VisualDetector.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"

namespace PokemonAutomation{
    class CancellableScope;
    class VideoFeed;
namespace NintendoSwitch{
namespace PokemonFRLG{

// Detect a player's Pokemon that is currently battling (not fainted)
class BattlePokemonDetector : public StaticScreenDetector{
public:
    BattlePokemonDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    // ImageFloatBox m_left_box;
    ImageFloatBox m_right_box;
    // ImageFloatBox m_top_box;
    ImageFloatBox m_bottom_box;
};

// Watches for the player's Pokemon to disappear
class BattleFaintWatcher : public DetectorToFinder<BattlePokemonDetector>{
public:
    BattleFaintWatcher(Color color)
        : DetectorToFinder("BattleFaintWatcher", FinderType::GONE, std::chrono::milliseconds(250), color)
    {}
};

// Detect an opposing Pokemon that is currently battling (not fainted)
class BattleOpponentDetector : public StaticScreenDetector{
public:
    BattleOpponentDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    ImageFloatBox m_left_box;
    ImageFloatBox m_right_box;
    // ImageFloatBox m_top_box;
    ImageFloatBox m_bottom_box;
};

// Watches for the opponent to disappear
class BattleOpponentFaintWatcher : public DetectorToFinder<BattleOpponentDetector>{
public:
    BattleOpponentFaintWatcher(Color color)
        : DetectorToFinder("BattleOPponentFaintWatcher", FinderType::GONE, std::chrono::milliseconds(250), color)
    {}
};





}
}
}

#endif
