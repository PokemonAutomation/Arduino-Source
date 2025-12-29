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
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonTools/ImageMatch/ImageMatchResult.h"
#include "CommonTools/ImageMatch/CroppedImageDictionaryMatcher.h"
#include "CommonTools/OCR/OCR_SmallDictionaryMatcher.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation{
    struct ProgramInfo;
namespace NintendoSwitch{
namespace PokemonLZA{

class DonutPowerReader : public OCR::SmallDictionaryMatcher{
public:
    static constexpr double MAX_LOG10P = -1.40;
    static constexpr double MAX_LOG10P_SPREAD = 0.50;

public:
    DonutPowerReader();

    static DonutPowerReader& instance();

    OCR::StringMatchResult read_substring(
        Logger& logger,
        Language language,
        const ImageViewRGB32& image,
        const std::vector<OCR::TextColorRange>& text_color_ranges,
        double min_text_ratio = 0.01, double max_text_ratio = 0.50
    ) const;

};


class DonutPowerDetector : public StaticScreenDetector{
public:
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
    ImageFloatBox m_box;
};

// Detect the quest in a given position (four positions before having to scroll)
class DonutPowerWatcher : public VisualInferenceCallback{
public:
    ~DonutPowerWatcher();
    DonutPowerWatcher(Logger& logger, Color color, VideoOverlay& overlay,  Language language, int position);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

    const std::string& quest_name() const { return m_quest_name; }


protected:
    VideoOverlay& m_overlay;
    DonutPowerDetector m_detector;
    std::string m_quest_name;
};

}
}
}
#endif
