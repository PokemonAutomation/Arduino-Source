/*  Hyperspace Calorie Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_HyperspaceCalorieDetector_H
#define PokemonAutomation_PokemonLZA_HyperspaceCalorieDetector_H


#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonTools/VisualDetector.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"

#include <list>
#include <utility>

namespace PokemonAutomation{
    class Logger;
namespace NintendoSwitch{
namespace PokemonLZA{


// Detects Calorie number in Hyperspace using number OCR.
class HyperspaceCalorieDetector : public StaticScreenDetector{
public:
    HyperspaceCalorieDetector(Logger& logger);

    virtual void make_overlays(VideoOverlaySet& items) const override;

    //  This is not const so that detectors can save/cache state.
    virtual bool detect(const ImageViewRGB32& screen) override;

    uint16_t calorie_number() const { return m_calorie_number; }

    // How many filters this detector uses to read Calorie numbers in parallel.
    // Call OCR::ensure_instances() in SerialPrograms/Source/CommonTools/OCR/OCR_RawOCR.h
    // to warm OCR object cache so it won't be slow the first time running the detector.
    static void warm_ocr();

private:
    Logger& m_logger;
    ImageFloatBox m_calorie_number_box;

    uint16_t m_calorie_number = 0;
};

// A watcher to run until it reads a valid OCR number.
class HyperspaceCalorieWatcher : public HyperspaceCalorieDetector, public VisualInferenceCallback{
public:
    HyperspaceCalorieWatcher(Logger& logger);

    virtual void make_overlays(VideoOverlaySet& items) const override;

    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
};


// A watcher to stop the inference session when a target Calorie limit is OCRed.
// It has some robustness logic to ensure occasional wrong number OCR won't
// lead to an early stop of the inference session and also saves potential
// screenshots of wrong OCR to debug folder.
class HyperspaceCalorieLimitWatcher : public HyperspaceCalorieDetector, public VisualInferenceCallback{
public:
    HyperspaceCalorieLimitWatcher(Logger& logger, uint16_t calorie_limit);

    virtual void make_overlays(VideoOverlaySet& items) const override;

    // Return true in `process_frame()` when the calorie limit is reached.
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    uint16_t m_calorie_limit;
    WallClock m_start_of_detection = WallClock::min();
    
    std::list<std::pair<uint16_t, ImageRGB32>> m_last_calorie_images;
};




}
}
}
#endif
