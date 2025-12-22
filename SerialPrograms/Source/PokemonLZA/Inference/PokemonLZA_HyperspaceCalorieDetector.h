/*  Hyperspace Calorie Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_HyperspaceCalorieDetector_H
#define PokemonAutomation_PokemonLZA_HyperspaceCalorieDetector_H

//#include <optional>
#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/VisualDetector.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"

namespace PokemonAutomation{
    class Logger;
namespace NintendoSwitch{
namespace PokemonLZA{


class HyperspaceCalorieDetector : public StaticScreenDetector{
public:
    HyperspaceCalorieDetector(Logger& logger);

    virtual void make_overlays(VideoOverlaySet& items) const override;

    //  This is not const so that detectors can save/cache state.
    virtual bool detect(const ImageViewRGB32& screen) override;

    uint16_t calorie_number() const { return m_calorie_number; }

private:
    Logger& m_logger;
    ImageFloatBox m_calorie_number_box;

    uint16_t m_calorie_number = 0;
};

class HyperspaceCalorieLimitWatcher : public HyperspaceCalorieDetector, public VisualInferenceCallback{
public:
    HyperspaceCalorieLimitWatcher(Logger& logger, uint16_t calorie_limit);

    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    uint16_t m_calorie_limit;
    WallClock m_start_of_detection = WallClock::min();
};




}
}
}
#endif
