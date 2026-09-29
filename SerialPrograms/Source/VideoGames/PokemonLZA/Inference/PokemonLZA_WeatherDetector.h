/*  Weather Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_WeatherDetector_H
#define PokemonAutomation_PokemonLZA_WeatherDetector_H

#include <optional>
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{


enum class WeatherIconType{
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
class WeatherIconDetector : public StaticScreenDetector{
public:
    WeatherIconDetector(WeatherIconType type, VideoOverlay* overlay = nullptr);

    virtual void make_overlays(VideoOverlaySet& items) const override;

    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    ImageFloatBox m_box;
    WeatherIconType m_type;

    std::optional<OverlayBoxScope> m_overlay1;
};


void add_tests_WeatherDetector(UnitTestDatabase& database);


}
}
}
#endif
