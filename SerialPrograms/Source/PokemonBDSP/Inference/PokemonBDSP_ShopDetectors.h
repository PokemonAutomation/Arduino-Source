/*  Shop Detectors
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_ShopDetectors_H
#define PokemonAutomation_PokemonBDSP_ShopDetectors_H

#include <string>
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Language.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "CommonTools/VisualDetector.h"
#include "PokemonBDSP_DialogDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


// Detects the blue decorative border at the bottom of the screen that appears
// when the mart buying list is open.
class ShopBuyingWindowDetector : public StaticScreenDetector{
public:
    // box: a thin strip covering the blue bottom border (user fills in coordinates).
    ShopBuyingWindowDetector(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    Color m_color;
    ImageFloatBox m_box;
};
class ShopBuyingWindowWatcher : public DetectorToFinder<ShopBuyingWindowDetector>{
public:
    ShopBuyingWindowWatcher(Color color = COLOR_RED)
        : DetectorToFinder("ShopBuyingWindowWatcher", std::chrono::milliseconds(250), color)
    {}
};


// Detects the quantity-input overlay (yellow/gold box + ShortDialog) that appears
// after selecting an item to buy.
class ShopQuantityWindowDetector : public StaticScreenDetector{
public:
    // box: the area containing the yellow quantity-input box (user fills in coordinates).
    ShopQuantityWindowDetector(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    Color m_color;
    ShortDialogDetector m_dialog;
    ImageFloatBox m_qty_box;
};
class ShopQuantityWindowWatcher : public DetectorToFinder<ShopQuantityWindowDetector>{
public:
    ShopQuantityWindowWatcher(Color color = COLOR_RED)
        : DetectorToFinder("ShopQuantityWindowWatcher", std::chrono::milliseconds(250), color)
    {}
};


// Scans `scan_area` to dynamically locate the red selection outline, then
// verifies the highlighted item by running OCR on the item name and price
// regions within the row and comparing against the expected `item_slug`.
// Fires only when the outline is found AND both OCR checks pass.
class ShopItemHoverDetector : public VisualInferenceCallback{
public:
    // scan_area: normalized screen region that covers the entire shop item list.
    // item_slug: identifies the expected item (e.g. "full-restore"). Must match
    //            a key in ShopItemNameOCR.json and have a known price in the
    //            internal lookup table.
    // language:  language used for OCR dictionary lookup.
    ShopItemHoverDetector(
        Logger& logger,
        const ImageFloatBox& scan_area,
        const std::string& item_slug,
        Language language = Language::English,
        Color color = COLOR_CYAN
    );

    bool detect(const ImageViewRGB32& screen);

    // Returns the last outline box confirmed by detect() / process_frame().
    // Valid only after a successful detection; width == 0 if nothing detected yet.
    const ImageFloatBox& detected_box() const { return m_detected_box; }

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    // Scans scan_area for a horizontal band of red edges (left + right) that
    // indicate the selection outline. Fills out_box with the outline's screen-
    // normalized bounds and returns true when found.
    bool find_outline(const ImageViewRGB32& screen, ImageFloatBox& out_box) const;

    // Runs OCR on the name and price sub-regions of outline_box.
    // Returns true when both match the expected item slug and its price.
    bool match_ocr(const ImageViewRGB32& screen, const ImageFloatBox& outline_box) const;

    Logger& m_logger;
    Color m_color;
    Language m_language;
    ImageFloatBox m_scan_area;
    std::string m_item_slug;
    std::string m_price_slug;
    mutable ImageFloatBox m_detected_box{0, 0, 0, 0};
    mutable ImageFloatBox m_last_name_box{0, 0, 0, 0};
    mutable ImageFloatBox m_last_price_box{0, 0, 0, 0};
    mutable bool m_ocr_logged = false;
    size_t m_trigger_count = 0;
    size_t m_frame_count   = 0;
};


}
}
}
#endif
