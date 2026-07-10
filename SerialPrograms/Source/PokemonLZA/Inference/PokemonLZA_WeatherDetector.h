#ifndef PokemonAutomation_PokemonLZA_WeatherDetector_H
#define PokemonAutomation_PokemonLZA_WeatherDetector_H

#include <optional>
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonLZA {


enum class WeatherIconType {
    Clear,
    Sunny,
    Rain,
    Cloudy,
    Foggy,
    Rainbow,
    Unknown,
};

//-----------------------------------------------------
//  Detector
//-----------------------------------------------------
class WeatherIconDetector : public StaticScreenDetector {
public:
    WeatherIconDetector(WeatherIconType type, VideoOverlay* overlay = nullptr);

    virtual void make_overlays(VideoOverlaySet& items) const override;

    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    WeatherIconType m_type;

    std::optional<OverlayBoxScope> m_overlay1;
};


}
}
}
#endif
