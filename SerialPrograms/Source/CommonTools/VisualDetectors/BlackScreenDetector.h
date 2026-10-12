/*  Black Screen Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *
 *      Returns true after a black screen has been detected and
 * the black screen has ended.
 *
 */

#ifndef PokemonAutomation_CommonTools_BlackScreenDetector_H
#define PokemonAutomation_CommonTools_BlackScreenDetector_H

#include <atomic>
#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation{


class BlackScreenDetector : public StaticScreenDetector{
public:
    BlackScreenDetector(
        Color color = COLOR_RED,
        const ImageFloatBox& box = {0.1, 0.1, 0.8, 0.8},
        double max_rgb_sum = 100,
        double max_stddev_sum = 10
    );

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    Color m_color;
    ImageFloatBox m_box;
    double m_max_rgb_sum;
    double m_max_stddev_sum;
};
class WhiteScreenDetector : public StaticScreenDetector{
public:
    WhiteScreenDetector(
        Color color = COLOR_RED,
        const ImageFloatBox& box = {0.1, 0.1, 0.8, 0.8},
        double min_rgb_sum = 500,
        double max_stddev_sum = 10
    );

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    Color m_color;
    ImageFloatBox m_box;
    double m_min_rgb_sum;
    double m_max_stddev_sum;
};


class BlackScreenWatcher : public DetectorToFinder<BlackScreenDetector>{
public:
    BlackScreenWatcher(
        Color color = COLOR_RED,
        const ImageFloatBox& box = {0.1, 0.1, 0.8, 0.8},
        double max_rgb_sum = 100,
        double max_stddev_sum = 10,
        FinderType finder_type = FinderType::PRESENT,
        std::chrono::milliseconds duration = std::chrono::milliseconds(100)
    )
        : DetectorToFinder("BlackScreenWatcher", finder_type, duration, color, box, max_rgb_sum, max_stddev_sum)
    {}
};

// Detect when a period of black screen is over
class BlackScreenOverWatcher : public VisualInferenceCallback{
public:
    BlackScreenOverWatcher(
        Color color = COLOR_RED,
        const ImageFloatBox& box = {0.1, 0.1, 0.8, 0.8},
        double max_rgb_sum = 100,
        double max_stddev_sum = 10,
        std::chrono::milliseconds hold_duration = std::chrono::milliseconds(100),
        std::chrono::milliseconds release_duration = std::chrono::milliseconds(100)
    );

    bool black_is_over(const ImageViewRGB32& frame);

    virtual void make_overlays(VideoOverlaySet& items) const override;

    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    BlackScreenWatcher m_on;
    BlackScreenWatcher m_off;
    bool m_has_been_black = false;
    std::atomic<bool> m_black_is_over = false;
};




class WhiteScreenOverWatcher : public VisualInferenceCallback{
public:
    WhiteScreenOverWatcher(
        Color color = COLOR_RED,
        const ImageFloatBox& box = {0.1, 0.1, 0.8, 0.8},
        double min_rgb_sum = 500,
        double max_stddev_sum = 10
    );

    bool white_is_over(const ImageViewRGB32& frame);

    virtual void make_overlays(VideoOverlaySet& items) const override;

    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    WhiteScreenDetector m_detector;
    bool m_has_been_white = false;
};



}
#endif
