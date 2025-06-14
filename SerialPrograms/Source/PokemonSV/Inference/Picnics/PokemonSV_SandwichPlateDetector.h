/*  Sandwich Plate Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_SandwichPlateDetector_H
#define PokemonAutomation_PokemonSV_SandwichPlateDetector_H

#include "Common/Cpp/AbstractLogger.h"
#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "CommonFramework/Language.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation{

class VideoOverlaySet;
class VideoOverlay;
class OverlayBoxScope;

namespace NintendoSwitch{
namespace PokemonSV{


// Detect the left, middle or right plate that hold sandwich fillings during the making sandwich minigame,
// using OCR to find the filling name tag under the plate.
class SandwichPlateDetector : public StaticScreenDetector{
public:
    enum class Side{
        LEFT,
        MIDDLE,
        RIGHT,
        NOT_APPLICABLE,
    };
    SandwichPlateDetector(Logger& logger, Color color, Language language, Side side);
    virtual ~SandwichPlateDetector();

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

    // Return detected sandwich filling name slug. Return empty string if not detected
    std::string detect_filling_name(const ImageViewRGB32& screen) const;

    // check if the filling name label is yellow.
    // It is yellow when a filling is grabbed by the sandwich hand in the minigame.
    bool is_label_yellow(const ImageViewRGB32& screen) const;

protected:
    Logger& m_logger;
    Color m_color;
    Language m_language;
    Side m_side;
    ImageFloatBox m_box;
};


// Detect the left, middle or right plate that hold sandwich fillings during the making sandwich minigame,
// using OCR to find the filling name tag under the plate.
class SandwichPlateWatcher : public VisualInferenceCallback{
public:
    using Side = SandwichPlateDetector::Side;
    ~SandwichPlateWatcher();
    SandwichPlateWatcher(Logger& logger, Color color, VideoOverlay& overlay,  Language language, Side side);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

    const std::string& filling_name() const { return m_filling_name; }


protected:
    VideoOverlay& m_overlay;
    SandwichPlateDetector m_detector;
    std::string m_filling_name;
};




}
}
}
#endif
