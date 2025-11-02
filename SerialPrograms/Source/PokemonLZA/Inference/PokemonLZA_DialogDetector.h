/*  Dialog Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_DialogDetector_H
#define PokemonAutomation_PokemonLZA_DialogDetector_H

#include <optional>
#include <atomic>
#include "Common/Cpp/Color.h"
#include "Common/Cpp/AbstractLogger.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/VisualDetector.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

//  Detect normal dialogue that is used in cases like when you talk to npcs in most situations.
//
//  Don't use this as there are lot of different dialogs. Use one of the newer
//  ones below. This was done using official screenshots before launch before
//  we knew there were so many different dialogs.
//
class NormalDialogDetector : public VisualInferenceCallback{
public:
    NormalDialogDetector(Logger& logger, VideoOverlay& overlay, bool stop_on_detected);

    bool detected() const{
        return m_detected.load(std::memory_order_acquire);
    }

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    bool m_stop_on_detected;
    std::atomic<bool> m_detected;
    ImageFloatBox m_title_green_line_box;
    ImageFloatBox m_black_arrow_box;
};




// Common white dialog box
class FlatWhiteDialogDetector : public StaticScreenDetector{
public:
    FlatWhiteDialogDetector(Color color = COLOR_RED, VideoOverlay* overlay = nullptr);

    virtual void make_overlays(VideoOverlaySet& items) const override;

    //  This is not const so that detectors can save/cache state.
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    friend class FlatWhiteDialogWatcher;

    const Color m_color;
    VideoOverlay* m_overlay;
    const ImageFloatBox m_bottom;
    const ImageFloatBox m_arrow_box;

    ImageFloatBox m_last_detected;
    std::optional<OverlayBoxScope> m_last_detected_box;
};
class FlatWhiteDialogWatcher : public DetectorToFinder<FlatWhiteDialogDetector>{
public:
    FlatWhiteDialogWatcher(
        Color color = COLOR_RED,
        VideoOverlay* overlay = nullptr,
        std::chrono::milliseconds hold_duration = std::chrono::milliseconds(250)
    )
         : DetectorToFinder("FlatWhiteDialogWatcher", hold_duration, color, overlay)
    {}
};


// Blue dialog box for when receiving an item
class BlueDialogDetector : public StaticScreenDetector{
public:
    BlueDialogDetector(Color color = COLOR_RED, VideoOverlay* overlay = nullptr);

    virtual void make_overlays(VideoOverlaySet& items) const override;

    //  This is not const so that detectors can save/cache state.
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    friend class BlueDialogWatcher;

    const Color m_color;
    VideoOverlay* m_overlay;
    const ImageFloatBox m_corner;
    const ImageFloatBox m_arrow_box;

    ImageFloatBox m_last_detected;
    std::optional<OverlayBoxScope> m_last_detected_box;
};
class BlueDialogWatcher : public DetectorToFinder<BlueDialogDetector>{
public:
    BlueDialogWatcher(
        Color color = COLOR_RED,
        VideoOverlay* overlay = nullptr,
        std::chrono::milliseconds hold_duration = std::chrono::milliseconds(250)
    )
         : DetectorToFinder("BlueDialogWatcher", hold_duration, color, overlay)
    {}
};


// A blue dialog box slightly bigger than BlueDialogDetector with item profile picture
// that appears after BlueDialogDetector to show the item description.
class ItemReceiveDetector : public StaticScreenDetector{
public:
    ItemReceiveDetector(Color color = COLOR_RED, VideoOverlay* overlay = nullptr);

    virtual void make_overlays(VideoOverlaySet& items) const override;

    //  This is not const so that detectors can save/cache state.
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    friend class BlueDialogWatcher;

    const Color m_color;
    VideoOverlay* m_overlay;
    const ImageFloatBox m_arrow_box;

    ImageFloatBox m_last_detected;
    std::optional<OverlayBoxScope> m_last_detected_box;
};
class ItemReceiveWatcher : public DetectorToFinder<ItemReceiveDetector>{
public:
    ItemReceiveWatcher(
        Color color = COLOR_RED,
        VideoOverlay* overlay = nullptr,
        std::chrono::milliseconds hold_duration = std::chrono::milliseconds(250)
    )
         : DetectorToFinder("ItemReceiveWatcher", hold_duration, color, overlay)
    {}
};



class TealDialogDetector : public StaticScreenDetector{
public:
    TealDialogDetector(Color color = COLOR_RED, VideoOverlay* overlay = nullptr);

    virtual void make_overlays(VideoOverlaySet& items) const override;

    //  This is not const so that detectors can save/cache state.
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    friend class TealDialogWatcher;

    const Color m_color;
    VideoOverlay* m_overlay;
    const ImageFloatBox m_arrow_box;

    ImageFloatBox m_last_detected;
    std::optional<OverlayBoxScope> m_last_detected_box;
};
class TealDialogWatcher : public DetectorToFinder<TealDialogDetector>{
public:
    TealDialogWatcher(
        Color color = COLOR_RED,
        VideoOverlay* overlay = nullptr,
        std::chrono::milliseconds hold_duration = std::chrono::milliseconds(250)
    )
         : DetectorToFinder("TealDialogWatcher", hold_duration, color, overlay)
    {}
};





}
}
}
#endif
