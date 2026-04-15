#pragma once

#include <optional>
#include "Common/Cpp/Color.h"
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

    DayNightStateDetector(VideoOverlay* overlay = nullptr);

    virtual void make_overlays(VideoOverlaySet& items) const override;

    virtual bool detect(const ImageViewRGB32& screen) override;

    DayNightState state() const;

private:

    ImageFloatBox m_box;

    DayNightState m_state;

};


}
}
}