/*  Donut Power Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_DonutPowerDetector_H
#define PokemonAutomation_PokemonLZA_DonutPowerDetector_H

#include <array>
#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Language.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation{
    struct ProgramInfo;
    class Logger;
namespace NintendoSwitch{
namespace PokemonLZA{


class DonutPowerDetector : public StaticScreenDetector{
public:
    // position: 0, 1 or 2. We have at most three powers on one donut.
    DonutPowerDetector(Logger& logger, Color color, Language language, int position);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

    // Return detected power string. Return empty string if not detected.
    // Return empty string if no power text found.
    std::string detect_power(const ImageViewRGB32& screen) const;

protected:
    Logger& m_logger;
    Color m_color;
    Language m_language;
    int m_position;
    ImageFloatBox m_ocr_box;
};

// Detect the quest in a given position
class DonutPowerWatcher : public VisualInferenceCallback{
public:
    ~DonutPowerWatcher();
    DonutPowerWatcher(Logger& logger, Color color, Language language, int position);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

    const std::string& quest_name() const { return m_quest_name; }


protected:
    DonutPowerDetector m_detector;
    std::string m_quest_name;
};

}
}
}
#endif
