/*  Shop Detectors
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <string>
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "CommonTools/OCR/OCR_NumberReader.h"
#include "CommonTools/OCR/OCR_Routines.h"
#include "PokemonBDSP_ShopItemNameReader.h"
#include "PokemonBDSP_ShopDetectors.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


// ---------------------------------------------------------------------------
//  ShopBuyingWindowDetector
// ---------------------------------------------------------------------------

// TODO: Fill in the actual coordinates of the blue bottom border strip.
// At 1280x720, the buying-list bottom bar appears near y ≈ 0.93.
// The colour ratio below matches BoxDetector's bottom bar ({0.190, 0.327, 0.482}).
static const ImageFloatBox SHOP_BUYING_WINDOW_BOX{0.020000, 0.960000, 0.750000, 0.035000};
static const FloatPixel    SHOP_BLUE_RATIO{0.190, 0.327, 0.483};
static constexpr double    SHOP_BLUE_MAX_DISTANCE = 0.10;
static constexpr double    SHOP_BLUE_MAX_STDDEV   = 15.0;

ShopBuyingWindowDetector::ShopBuyingWindowDetector(Color color)
    : m_color(color)
    , m_box(SHOP_BUYING_WINDOW_BOX)
{}
void ShopBuyingWindowDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}
bool ShopBuyingWindowDetector::detect(const ImageViewRGB32& screen){
    ImageStats stats = image_stats(extract_box_reference(screen, m_box));
    return is_solid(stats, SHOP_BLUE_RATIO, SHOP_BLUE_MAX_DISTANCE, SHOP_BLUE_MAX_STDDEV);
}


// ---------------------------------------------------------------------------
//  ShopQuantityWindowDetector
// ---------------------------------------------------------------------------

// TODO: Fill in the coordinates of the yellow quantity-input box.
// It typically appears in the centre-right of the screen when selecting quantity.
static const ImageFloatBox SHOP_QUANTITY_BOX{0.800000, 0.570000, 0.100000, 0.140000};
// Expected colour ratio for the gold/yellow quantity box.
// High R, high G, low B — adjust if the box colour looks different in your capture.
static const FloatPixel    SHOP_YELLOW_RATIO{0.58, 0.38, 0.04};
static constexpr double    SHOP_YELLOW_MAX_DISTANCE = 0.20;
static constexpr double    SHOP_YELLOW_MAX_STDDEV   = 20.0;

ShopQuantityWindowDetector::ShopQuantityWindowDetector(Color color)
    : m_color(color)
    , m_dialog(color)
    , m_qty_box(SHOP_QUANTITY_BOX)
{}
void ShopQuantityWindowDetector::make_overlays(VideoOverlaySet& items) const{
    m_dialog.make_overlays(items);
    items.add(m_color, m_qty_box);
}
bool ShopQuantityWindowDetector::detect(const ImageViewRGB32& screen){
    if (!m_dialog.detect(screen)){
        return false;
    }
    ImageStats stats = image_stats(extract_box_reference(screen, m_qty_box));
    return is_solid(stats, SHOP_YELLOW_RATIO, SHOP_YELLOW_MAX_DISTANCE, SHOP_YELLOW_MAX_STDDEV);
}


// ---------------------------------------------------------------------------
//  ShopItemHoverDetector
// ---------------------------------------------------------------------------

// The green description bar repeats the selected item name in white, just after its icon.
static const ImageFloatBox SHOP_DESCRIPTION_NAME_BOX{0.456, 0.754, 0.280, 0.042};
static constexpr size_t HOVER_MIN_CONSECUTIVE = 3;

ShopItemHoverDetector::ShopItemHoverDetector(
    Logger& logger,
    const std::string& item_slug,
    Language language,
    Color color
)
    : VisualInferenceCallback("ShopItemHoverDetector")
    , m_logger(logger)
    , m_color(color)
    , m_language(language)
    , m_item_slug(item_slug)
{}

bool ShopItemHoverDetector::detect(const ImageViewRGB32& screen){
    const auto result = ShopItemNameReader::instance().match_substring_from_image_multifiltered(
        nullptr, m_language, extract_box_reference(screen, SHOP_DESCRIPTION_NAME_BOX),
        OCR::WHITE_TEXT_FILTERS(), ShopItemNameReader::MAX_LOG10P, ShopItemNameReader::MAX_LOG10P_SPREAD
    );
    if (result.results.empty() || result.results.begin()->second.token != m_item_slug){
        m_ocr_logged = false;
        return false;
    }
    if (!m_ocr_logged){
        result.log(m_logger, ShopItemNameReader::MAX_LOG10P, "want: " + m_item_slug);
        m_ocr_logged = true;
    }
    return true;
}

int ShopItemHoverDetector::read_stock(const ImageViewRGB32& screen){
    if (!detect(screen)){
        return -1;
    }
    // Bag count at the lower right of the item-description panel; exclude the x.
    const ImageFloatBox stock_box{0.935, 0.855, 0.050, 0.065};
    const int stock = OCR::read_number(m_logger, extract_box_reference(screen, stock_box));
    return stock >= 0 && stock <= 999 ? stock : -1;
}

void ShopItemHoverDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, SHOP_DESCRIPTION_NAME_BOX);
}

bool ShopItemHoverDetector::process_frame(const ImageViewRGB32& frame, WallClock /*timestamp*/){
    // Only run OCR every 5th frame to reduce CPU load.
    if (m_frame_count++ % 5 != 0){
        return false;
    }
    if (!detect(frame)){
        m_trigger_count = 0;
        return false;
    }
    m_trigger_count++;
    return m_trigger_count >= HOVER_MIN_CONSECUTIVE;
}


}
}
}
