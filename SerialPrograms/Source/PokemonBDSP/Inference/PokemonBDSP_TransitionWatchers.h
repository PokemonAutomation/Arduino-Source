#pragma once
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
namespace PokemonAutomation{ namespace NintendoSwitch{ namespace PokemonBDSP{
// AutoStory-specific transition handling; shared watchers retain upstream behavior.
class WhiteScreenWatcher : public DetectorToFinder<WhiteScreenDetector>{
public:
    WhiteScreenWatcher(
        Color color = COLOR_RED,
        const ImageFloatBox& box = {0.1, 0.1, 0.8, 0.8},
        double min_rgb_sum = 500,
        double max_stddev_sum = 10,
        FinderType finder_type = FinderType::PRESENT,
        std::chrono::milliseconds duration = std::chrono::milliseconds(100)
    )
        : DetectorToFinder("WhiteScreenWatcher", finder_type, duration, color, box, min_rgb_sum, max_stddev_sum)
    {}
};

class BlackScreenFlashWatcher : public BlackScreenOverWatcher{
public:
    BlackScreenFlashWatcher(
        Color color = COLOR_RED,
        const ImageFloatBox& box = {0.1, 0.1, 0.8, 0.8},
        double max_rgb_sum = 100,
        double max_stddev_sum = 10,
        std::chrono::milliseconds hold_duration = std::chrono::milliseconds(20),
        std::chrono::milliseconds release_duration = std::chrono::milliseconds(100)
    )
        : BlackScreenOverWatcher(color, box, max_rgb_sum, max_stddev_sum, hold_duration, release_duration)
    {}
};

class WhiteScreenOverWatcher : public VisualInferenceCallback{
public:
    WhiteScreenOverWatcher(
        Color color = COLOR_RED,
        const ImageFloatBox& box = {0.1, 0.1, 0.8, 0.8},
        double min_rgb_sum = 500,
        double max_stddev_sum = 10,
        std::chrono::milliseconds hold_duration = std::chrono::milliseconds(100),
        std::chrono::milliseconds release_duration = std::chrono::milliseconds(100)
    );

    bool white_is_over(const ImageViewRGB32& frame);

    virtual void make_overlays(VideoOverlaySet& items) const override;

    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    WhiteScreenWatcher m_on;
    WhiteScreenWatcher m_off;
    bool m_has_been_white = false;
    std::atomic<bool> m_white_is_over = false;
};
}}}
