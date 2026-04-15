#pragma once

#include <optional>

#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

enum class DayNightState{
    DAY,
    NIGHT
};

class DayNightStateDetector : public StaticScreenDetector{
public:
    DayNightStateDetector();

    virtual void make_overlays(VideoOverlaySet& items) const override;

    virtual bool detect(const ImageViewRGB32& screen) override;

    DayNightState state() const{
        return m_state;
    }

private:

    ImageFloatBox m_box;

    DayNightState m_state;

    // persistent overlay box (WeatherDetector style)
    mutable std::optional<OverlayBoxScope> m_overlay;
};

}
}
}