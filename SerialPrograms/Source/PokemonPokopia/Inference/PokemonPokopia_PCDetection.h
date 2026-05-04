/*  PC Detection
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonPokopia_PCDetection_H
#define PokemonAutomation_PokemonPokopia_PCDetection_H

#include <optional>
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonPokopia{

// The stamp locations actually move week to week, get the locations dynamically
// const ImageFloatBox STAMP1_BOX{0.057000, 0.162500, 0.167000, 0.300000};
// const ImageFloatBox STAMP2_BOX{0.109000, 0.568000, 0.167000, 0.300000};
// const ImageFloatBox STAMP3_BOX{0.257000, 0.291000, 0.167000, 0.300000};
// const ImageFloatBox STAMP4_BOX{0.396500, 0.570000, 0.167000, 0.300000};
// const ImageFloatBox STAMP5_BOX{0.496000, 0.206000, 0.167000, 0.300000};
const ImageFloatBox TODAYS_STAMP_BOX{0.759000, 0.322000, 0.167000, 0.300000};

// TODO: Add Mewtwo and legendary bird stamps
enum class Stamp{
    MEW,
    OTHER // Other includes no stamp for now
};

class InfoIconDetector : public StaticScreenDetector{
public:
    InfoIconDetector(
        Color color,
        VideoOverlay* overlay,
        const ImageFloatBox& box
    );

    const ImageFloatBox& last_detected() const { return m_last_detected; }

    virtual void make_overlays(VideoOverlaySet& items) const override;

    //  This is not const so that detectors can save/cache state.
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    friend class InfoIconWatcher;

    const Color m_color;
    VideoOverlay* m_overlay;
    const ImageFloatBox m_arrow_box;

    ImageFloatBox m_last_detected;
    std::optional<OverlayBoxScope> m_last_detected_box;
};
class InfoIconWatcher : public DetectorToFinder<InfoIconDetector>{
public:
    InfoIconWatcher(
        Color color,
        VideoOverlay* overlay,
        const ImageFloatBox& box,
        std::chrono::milliseconds hold_duration = std::chrono::milliseconds(250)
    )
         : DetectorToFinder("InfoIconWatcher", hold_duration, color, overlay, box)
    {}
};

class StampMatcher;

class StampDetector : public StaticScreenDetector{
public:
    StampDetector(
        Color color,
        Stamp stamp,
        const ImageFloatBox& box,
        VideoOverlay* overlay = nullptr
    );
    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

    virtual void reset_state() override { m_last_detected_box.reset(); }

    Stamp stamp() const { return m_stamp; }

private:
    Stamp m_stamp;
    Color m_color;
    const StampMatcher& m_matcher;
    ImageFloatBox m_box;
    VideoOverlay* m_overlay;

    ImageFloatBox m_last_detected;
    std::optional<OverlayBoxScope> m_last_detected_box;
};
class StampWatcher : public DetectorToFinder<StampDetector>{
public:
    StampWatcher(
        Color color,
        Stamp stamp,
        const ImageFloatBox& box,
        VideoOverlay* overlay = nullptr,
        std::chrono::milliseconds hold_duration = std::chrono::milliseconds(250)
    )
         : DetectorToFinder("StampWatcher", hold_duration, color, stamp, box, overlay)
    {}
};

enum class RecipeType{
    SINGLE,
    DOUBLE,
    TRIPLE,
    QUAD,
    NOT_RECIPE
};

class RecipeIconDetector : public StaticScreenDetector{
public:
    RecipeIconDetector(
        Color color,
        VideoOverlay* overlay,
        const ImageFloatBox& box
    );

    const ImageFloatBox& last_detected() const { return m_last_detected; }
    const RecipeType& recipe_type() const { return m_recipe_type; }

    virtual void make_overlays(VideoOverlaySet& items) const override;

    //  This is not const so that detectors can save/cache state.
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    friend class RecipeWatcher;

    const Color m_color;
    VideoOverlay* m_overlay;
    const ImageFloatBox m_arrow_box;

    RecipeType m_recipe_type;
    ImageFloatBox m_last_detected;
    std::optional<OverlayBoxScope> m_last_detected_box;
};
class RecipeIconWatcher : public DetectorToFinder<RecipeIconDetector>{
public:
    RecipeIconWatcher(
        Color color,
        VideoOverlay* overlay,
        const ImageFloatBox& box,
        std::chrono::milliseconds hold_duration = std::chrono::milliseconds(250)
    )
         : DetectorToFinder("RecipeIconWatcher", hold_duration, color, overlay, box)
    {}
};

class CoinCountDetector : public StaticScreenDetector{
public:
    CoinCountDetector(Logger& logger);

    virtual void make_overlays(VideoOverlaySet& items) const override;

    //  This is not const so that detectors can save/cache state.
    virtual bool detect(const ImageViewRGB32& screen) override;

    uint32_t coin_count() const { return m_coin_count; }

private:
    Logger& m_logger;
    ImageFloatBox m_coin_count_box;

    uint32_t m_coin_count = 0;
};

class CoinCountWatcher : public CoinCountDetector, public VisualInferenceCallback{
public:
    CoinCountWatcher(Logger& logger);

    virtual void make_overlays(VideoOverlaySet& items) const override;

    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;
};

class CoinIconDetector : public StaticScreenDetector{
public:
    CoinIconDetector(
        Color color,
        VideoOverlay* overlay,
        const ImageFloatBox& box
    );

    const ImageFloatBox& last_detected() const { return m_last_detected; }

    virtual void make_overlays(VideoOverlaySet& items) const override;

    //  This is not const so that detectors can save/cache state.
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    friend class CoinIconWatcher;

    const Color m_color;
    VideoOverlay* m_overlay;
    const ImageFloatBox m_arrow_box;

    ImageFloatBox m_last_detected;
    std::optional<OverlayBoxScope> m_last_detected_box;
};
class CoinIconWatcher : public DetectorToFinder<CoinIconDetector>{
public:
    CoinIconWatcher(
        Color color,
        VideoOverlay* overlay,
        const ImageFloatBox& box,
        std::chrono::milliseconds hold_duration = std::chrono::milliseconds(250)
    )
         : DetectorToFinder("CoinIconWatcher", hold_duration, color, overlay, box)
    {}
};



}
}
}
#endif
