/*  ESP Emotion Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_ESPEmotionDetector_H
#define PokemonAutomation_PokemonSV_ESPEmotionDetector_H

#include <atomic>
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation{
    class CancellableScope;
    class VideoFeed;
namespace NintendoSwitch{
namespace PokemonSV{

enum class Detection{
    NO_DETECTION,
    RED,
    YELLOW,
    BLUE,
    GREEN,
    GREY,
};
class ESPEmotionReader{
public:
    ESPEmotionReader();

    void make_overlays(VideoOverlaySet& items) const;
    Detection detect(const ImageViewRGB32& screen) const;

private:
    ImageFloatBox m_symbol_box;
};

class ESPEmotionDetector : public VisualInferenceCallback{
public:
    ESPEmotionDetector();

    Detection result() const{
        return m_last.load(std::memory_order_acquire);
    }

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    ESPEmotionReader m_reader;
    std::atomic<Detection> m_last;
};

//This checks for the Dendra's dialog box
class ESPStartDetector : public VisualInferenceCallback{
public:
    ESPStartDetector();

    bool detect(const ImageViewRGB32& frame);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    ImageFloatBox m_left_box;
    ImageFloatBox m_right_box;
};

// This checks that Dendra's box vanished to shout the next emotion
class ESPShowNewEmotionDetector : public VisualInferenceCallback{
public:
    ESPShowNewEmotionDetector();

    bool detect(const ImageViewRGB32& frame);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    ImageFloatBox m_left_box;
    ImageFloatBox m_right_box;
};

// Check that an emotion is pressed by looking for yellow in the lower right interface
class ESPPressedEmotionDetector : public VisualInferenceCallback{
public:
    ESPPressedEmotionDetector();

    bool detect(const ImageViewRGB32& frame);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    ImageFloatBox m_left_box;
    ImageFloatBox m_right_box;
    ImageFloatBox m_top_box;
    ImageFloatBox m_bottom_box;
};


}
}
}

#endif
