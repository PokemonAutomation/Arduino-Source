/*  Tera Raid Search Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_TeraRaidSearchDetector_H
#define PokemonAutomation_PokemonSV_TeraRaidSearchDetector_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/InferenceInfra/VisualInferenceCallback.h"
#include "CommonFramework/Inference/VisualDetector.h"

namespace PokemonAutomation{
    class ConsoleHandle;
    class BotBaseContext;
    struct ProgramInfo;
namespace NintendoSwitch{
namespace PokemonSV{



class TeraRaidSearchDetector : public StaticScreenDetector{
public:
    TeraRaidSearchDetector(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

    bool detect_search_location(ImageFloatBox& box, const ImageViewRGB32& screen) const;
    bool move_cursor_to_search(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context) const;

private:
    Color m_color;
};
class TeraRaidSearchWatcher : public DetectorToFinder<TeraRaidSearchDetector>{
public:
    TeraRaidSearchWatcher(
        Color color = COLOR_RED,
        std::chrono::milliseconds duration = std::chrono::milliseconds(250)
    )
         : DetectorToFinder("TeraRaidSearchWatcher", duration, color)
    {}
};



class CodeEntryDetector : public StaticScreenDetector{
public:
    CodeEntryDetector(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

private:
    Color m_color;
    ImageFloatBox m_bottom;
    ImageFloatBox m_left;
    ImageFloatBox m_right;
    ImageFloatBox m_center;
};
class CodeEntryWatcher : public DetectorToFinder<CodeEntryDetector>{
public:
    CodeEntryWatcher(Color color = COLOR_RED)
         : DetectorToFinder("CodeEntryWatcher", std::chrono::milliseconds(250), color)
    {}
};



}
}
}
#endif
