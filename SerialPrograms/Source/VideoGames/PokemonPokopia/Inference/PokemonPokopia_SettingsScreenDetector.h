/*  Settings Screen Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonPokopia_SettingsScreenDetector_H
#define PokemonAutomation_PokemonPokopia_SettingsScreenDetector_H

#include <optional>
#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/VisualDetector.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonPokopia{


// Detector the solid boxes on the settings screen
class SettingsScreenDetector : public StaticScreenDetector{
public:
    SettingsScreenDetector(ImageFloatBox box);

    virtual void make_overlays(VideoOverlaySet& items) const override;

    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    const ImageFloatBox m_box;
};

class SettingsScreenWatcher : public DetectorToFinder<SettingsScreenDetector>{
public:
    SettingsScreenWatcher(
        ImageFloatBox box,
        std::chrono::milliseconds hold_duration = std::chrono::milliseconds(100)
    )
         : DetectorToFinder("SettingsScreenWatcher", hold_duration, box)
    {}
};



}
}
}
#endif