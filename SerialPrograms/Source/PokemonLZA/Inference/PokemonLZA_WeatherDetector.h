#ifndef PokemonAutomation_PokemonLZA_WeatherDetector_H
#define PokemonAutomation_PokemonLZA_WeatherDetector_H

#include <optional>
#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/VisualDetector.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"

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
//  Template Info Struct
//-----------------------------------------------------
struct WeatherTemplateInfo{
    const char* path1;          // template for box1
    const char* path2;          // template for box2
    ImageFloatBox box1;
    ImageFloatBox box2;
    Color color1;
    Color color2;
};


const WeatherTemplateInfo& weather_template_info(WeatherIconType icon);


//-----------------------------------------------------
//  Detector
//-----------------------------------------------------
class WeatherIconDetector : public StaticScreenDetector {
public:
    WeatherIconDetector(WeatherIconType type, VideoOverlay* overlay = nullptr);

    virtual void make_overlays(VideoOverlaySet& items) const override;

    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    const WeatherTemplateInfo* m_info;

    std::optional<OverlayBoxScope> m_overlay1;
    std::optional<OverlayBoxScope> m_overlay2;
};


}
}
}
#endif
