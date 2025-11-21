/*  Run From Battle Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_RunFromBattleDetector_H
#define PokemonAutomation_PokemonLZA_RunFromBattleDetector_H

#include "PokemonLZA/Inference/PokemonLZA_ButtonDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{




class RunFromBattleDetector : public StaticScreenDetector{
public:
    RunFromBattleDetector(Color color, VideoOverlay* overlay);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;


private:
//    Color m_color;
    ButtonDetector m_button;
};
class RunFromBattleWatcher : public DetectorToFinder<RunFromBattleDetector>{
public:
    RunFromBattleWatcher(
        Color color,
        VideoOverlay* overlay = nullptr,
        std::chrono::milliseconds hold_duration = std::chrono::milliseconds(100)
    )
         : DetectorToFinder("RunFromBattleWatcher", hold_duration, color, overlay)
    {}
};



}
}
}
#endif
