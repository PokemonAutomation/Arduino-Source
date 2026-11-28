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


// Reads the selected item name from the green shop description bar.
// process_frame() requires three consecutive sampled matches.
class ShopItemHoverDetector : public VisualInferenceCallback{
public:
    // item_slug: identifies the expected item (e.g. "full-restore"). Must match
    //            a key in the item-name resources loaded by ShopItemNameReader.
    // language:  language used for OCR dictionary lookup.
    ShopItemHoverDetector(
        Logger& logger,
        const std::string& item_slug,
        Language language = Language::English,
        Color color = COLOR_CYAN
    );

    bool detect(const ImageViewRGB32& screen);

    // Read the Bag stock count only when this item's selected name matches.
    // Returns -1 for an unrecognized screen or unreadable/out-of-range count.
    int read_stock(const ImageViewRGB32& screen);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    Logger& m_logger;
    Color m_color;
    Language m_language;
    std::string m_item_slug;
    bool m_ocr_logged = false;
    size_t m_trigger_count = 0;
    size_t m_frame_count   = 0;
};


}
}
}
#endif
