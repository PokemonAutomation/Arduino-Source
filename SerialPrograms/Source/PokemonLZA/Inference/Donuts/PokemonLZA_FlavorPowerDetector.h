/*  Flavor Power Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_FlavorPowerDetector_H
#define PokemonAutomation_PokemonLZA_FlavorPowerDetector_H

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


// Detect donut power icons
class FlavorPowerIconDetector{
public:
    FlavorPowerIconDetector(Logger& logger, int position);

    void make_overlays(VideoOverlaySet& items) const;

    // Currently the only implementation is to detect the flavor power level (1-3)
    // If no such power at this row, return -1.
    int detect(const ImageViewRGB32& screen);

private:
    Logger& m_logger;
    ImageFloatBox m_icon_box;
    ImageFloatBox m_empty_space_after_number;
    ImageFloatBox m_subtype_free_number_box;
    ImageFloatBox m_subtype_number_box;
};


// Use OCR to detect donut flavor power texts
class FlavorPowerDetector : public StaticScreenDetector{
public:
    // position: 0, 1 or 2. We have at most three powers on one donut.
    FlavorPowerDetector(Logger& logger, Color color, Language language, int position);

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
class FlavorPowerWatcher : public VisualInferenceCallback{
public:
    ~FlavorPowerWatcher();
    FlavorPowerWatcher(Logger& logger, Color color, Language language, int position);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

    const std::string& quest_name() const { return m_quest_name; }


protected:
    FlavorPowerDetector m_detector;
    std::string m_quest_name;
};

}
}
}
#endif
