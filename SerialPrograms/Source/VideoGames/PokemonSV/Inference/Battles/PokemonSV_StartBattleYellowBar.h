/*  Start Battle Yellow Bar
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_StartBattleYellowBar_H
#define PokemonAutomation_PokemonSV_StartBattleYellowBar_H

#include <atomic>
#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



class StartBattleYellowBarDetector : public StaticScreenDetector{
public:
    StartBattleYellowBarDetector(Color color);
    virtual void make_overlays(VideoOverlaySet& items) const override;

    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    Color m_color;
    ImageFloatBox m_top;
    ImageFloatBox m_bot;
    ImageFloatBox m_mid;
};
class StartBattleYellowBarWatcher : public StartBattleYellowBarDetector, public VisualInferenceCallback{
public:
    StartBattleYellowBarWatcher(Color color)
        : StartBattleYellowBarDetector(color)
        , VisualInferenceCallback("StartBattleYellowBarWatcher")
        , m_start_detection(WallClock::min())
        , m_last_detected(WallClock::min())
    {}

    WallClock last_detected() const{
        return m_last_detected.load(std::memory_order_relaxed);
    }

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    WallClock m_start_detection;
    std::atomic<WallClock> m_last_detected;
};




}
}
}
#endif
