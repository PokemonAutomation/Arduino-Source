/*  Detectors for various pokemon info on box view
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_BoxInfoDetector_H
#define PokemonAutomation_PokemonLZA_BoxInfoDetector_H

#include <optional>
#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/VisualDetector.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"

namespace PokemonAutomation{
    class Logger;
namespace NintendoSwitch{
namespace PokemonLZA{

// Detect shiny sparkles when viewing a Pokemon in the box system
class BoxShinyDetector : public StaticScreenDetector{
public:
    BoxShinyDetector(Color color = COLOR_RED, VideoOverlay* overlay = nullptr);

    virtual void make_overlays(VideoOverlaySet& items) const override;

    //  This is not const so that detectors can save/cache state.
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    friend class BoxShinyWatcher;

    Color m_color;
    ImageFloatBox m_box;
    VideoOverlay* m_overlay;

    ImageFloatBox m_last_detected;
    std::optional<OverlayBoxScope> m_last_detected_box;
};

// Detect whether there is a shiny symbol on the current pokemon in the box system.
// Call BoxShinyWatcher::consistent_result() to know if the pokemon is shiny or not.
class BoxShinyWatcher : public DetectorToFinder<BoxShinyDetector>{
public:
    BoxShinyWatcher(
        Color color = COLOR_RED,
        VideoOverlay* overlay = nullptr,
        std::chrono::milliseconds hold_duration = std::chrono::milliseconds(250)
    )
         : DetectorToFinder("BoxShinyWatcher", FinderType::CONSISTENT, hold_duration, color, overlay)
    {}
};

// Detect alpha symbol when viewing a Pokemon in the box system
class BoxAlphaDetector : public StaticScreenDetector{
public:
    BoxAlphaDetector(Color color = COLOR_RED, VideoOverlay* overlay = nullptr);

    virtual void make_overlays(VideoOverlaySet& items) const override;

    //  This is not const so that detectors can save/cache state.
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    friend class BoxAlphaWatcher;

    Color m_color;
    ImageFloatBox m_box;
    VideoOverlay* m_overlay;

    ImageFloatBox m_last_detected;
    std::optional<OverlayBoxScope> m_last_detected_box;
};

// Detect whether there is an alpha symbol on the current pokemon in the box system.
// Call BoxAlphaWatcher::consistent_result() to know if the pokemon is alpha or not.
class BoxAlphaWatcher : public DetectorToFinder<BoxAlphaDetector>{
public:
    BoxAlphaWatcher(
        Color color = COLOR_RED,
        VideoOverlay* overlay = nullptr,
        std::chrono::milliseconds hold_duration = std::chrono::milliseconds(250)
    )
         : DetectorToFinder("BoxAlphaWatcher", FinderType::CONSISTENT, hold_duration, color, overlay)
    {}
};

// Check both shiny and alpha-ness of the current pokemon in the box view
class BoxPageInfoWatcher : public VisualInferenceCallback{
public:
    BoxPageInfoWatcher(VideoOverlay* overlay);

    virtual void make_overlays(VideoOverlaySet& items) const override;

    // return true when the watcher determines the shiny and alpha-ness of the pokemon
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

    // reset internal state so the watcher is ready for detect another pokemon's info
    void reset_state(){
        m_shiny_watcher.reset_state();
        m_alpha_watcher.reset_state();
    }

    // After detection is done, call this to check shiny-ness
    bool is_shiny() const { return m_shiny_watcher.consistent_result(); }
    // After detection is done, call this to check alpha-ness
    bool is_alpha() const { return m_alpha_watcher.consistent_result(); }
    // Return string of detected shiny and alpha-ness for logging. It can be:
    // - "Regular"
    // - "Shiny"
    // - "Alpha"
    // - "Shiny Alpha"
    std::string info_str() const;

private:
    BoxShinyWatcher m_shiny_watcher;
    BoxAlphaWatcher m_alpha_watcher;
};


enum class DexType{
    LUMIOSE,
    HYPERSPACE,
};


class BoxDexNumberDetector : public StaticScreenDetector{
public:
    BoxDexNumberDetector(Logger& logger);

    virtual void make_overlays(VideoOverlaySet& items) const override;

    virtual bool detect(const ImageViewRGB32& screen) override;

    // Return either DexType::LUMIOSE or DexType::HYPERSPACE.
    DexType dex_type() const { return m_dex_type; }
    // Return dex number (1-indexed).
    uint16_t dex_number() const { return m_dex_number; }


    // Debugging value, used to compute whether the pokemon origin symbol is Lumiose or Hyperspace.
    double dex_type_color_ratio() const { return m_dex_type_color_ratio; }
    // Debugging value, if the detected dex number value is not a correct dex number, what number it is.
    int dex_number_when_error() const { return m_dex_number_when_error; }

private:
    friend class BoxShinyWatcher;

    Logger& m_logger;
    ImageFloatBox m_dex_number_box;
    ImageFloatBox m_dex_type_box;

    uint16_t m_dex_number = 0;
    DexType m_dex_type = DexType::LUMIOSE;
    double m_dex_type_color_ratio = 0.0;
    int m_dex_number_when_error = 0;
};


}
}
}
#endif
