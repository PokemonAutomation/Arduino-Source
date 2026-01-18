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
#include "Common/Cpp/Logger/AbstractLogger.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/VisualDetector.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{


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

    ImageFloatBox m_last_detected_box;
    std::optional<OverlayBoxScope> m_last_detected_box_scope;
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

    ImageFloatBox m_last_detected_box;
    std::optional<OverlayBoxScope> m_last_detected_box_scope;
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
    const ImageFloatBox m_top;
    const ImageFloatBox m_arrow_box;

    ImageFloatBox m_last_detected_box;
    std::optional<OverlayBoxScope> m_last_detected_box_scope;
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

    ImageFloatBox m_last_detected_box;
    std::optional<OverlayBoxScope> m_last_detected_box_scope;
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


// Transparent dialog box shows when entering a trainer battle 
class TransparentBattleDialogDetector : public StaticScreenDetector{
public:
    TransparentBattleDialogDetector(Color color = COLOR_RED, VideoOverlay* overlay = nullptr);

    virtual void make_overlays(VideoOverlaySet& items) const override;

    //  This is not const so that detectors can save/cache state.
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    friend class TransparentBattleDialogWatcher;

    const Color m_color;
    VideoOverlay* m_overlay;

    ImageFloatBox m_last_detected_box;
    std::optional<OverlayBoxScope> m_last_detected_box_scope;
};
class TransparentBattleDialogWatcher : public DetectorToFinder<TransparentBattleDialogDetector>{
public:
    TransparentBattleDialogWatcher(
        Color color = COLOR_RED,
        VideoOverlay* overlay = nullptr,
        std::chrono::milliseconds hold_duration = std::chrono::milliseconds(250)
    )
         : DetectorToFinder("TransparentBattleDialogWatcher", hold_duration, color, overlay)
    {}
};


// Light blue dialog box for holograms
class LightBlueDialogDetector : public StaticScreenDetector{
public:
    LightBlueDialogDetector(Color color = COLOR_RED, VideoOverlay* overlay = nullptr);

    virtual void make_overlays(VideoOverlaySet& items) const override;

    //  This is not const so that detectors can save/cache state.
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    friend class LightBlueDialogWatcher;

    const Color m_color;
    VideoOverlay* m_overlay;
    const ImageFloatBox m_corner;

    ImageFloatBox m_last_detected_box;
    std::optional<OverlayBoxScope> m_last_detected_box_scope;
};
class LightBlueDialogWatcher : public DetectorToFinder<LightBlueDialogDetector>{
public:
    LightBlueDialogWatcher(
        Color color = COLOR_RED,
        VideoOverlay* overlay = nullptr,
        std::chrono::milliseconds hold_duration = std::chrono::milliseconds(250)
    )
         : DetectorToFinder("BlueDialogWatcher", hold_duration, color, overlay)
    {}
};


}
}
}
#endif
