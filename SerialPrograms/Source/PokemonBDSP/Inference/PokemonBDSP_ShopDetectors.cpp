/*  Shop Detectors
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <string>
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonTools/Images/BinaryImage_FilterRgb32.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "CommonTools/OCR/OCR_NumberReader.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
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

// Color filter for the red hover rectangle.
static constexpr uint32_t RED_MIN = 0xff800000;  // R≥128, G=0,   B=0
static constexpr uint32_t RED_MAX = 0xffff6060;  // R≤255, G≤96,  B≤96

// Minimum pixel area for a red blob to be considered (filters noise).
static constexpr size_t RED_MIN_AREA = 200;

// The hover rectangle must span at least this fraction of the scan area width.
static constexpr double RED_MIN_WIDTH_RATIO = 0.80;

// Minimum aspect ratio (width/height) — the row is much wider than tall.
static constexpr double RED_MIN_ASPECT = 4.0;

// Minimum consecutive frames that must confirm before the detector fires.
static constexpr size_t HOVER_MIN_CONSECUTIVE = 3;

// Item-name OCR region, relative to the selected row.
static constexpr double NAME_X_OFFSET = 0.08;
static constexpr double NAME_X_WIDTH = 0.65;
static constexpr double OCR_ROW_HEIGHT = 0.06;

ShopItemHoverDetector::ShopItemHoverDetector(
    Logger& logger,
    const ImageFloatBox& scan_area,
    const std::string& item_slug,
    Language language,
    Color color
)
    : VisualInferenceCallback("ShopItemHoverDetector")
    , m_logger(logger)
    , m_color(color)
    , m_language(language)
    , m_scan_area(scan_area)
    , m_item_slug(item_slug)
{}

bool ShopItemHoverDetector::find_outline(const ImageViewRGB32& screen, ImageFloatBox& out_box) const{
    using namespace Kernels::Waterfill;

    const ImageViewRGB32 region = extract_box_reference(screen, m_scan_area);
    PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(region, RED_MIN, RED_MAX);

    auto session = make_WaterfillSession(matrix);
    auto iter    = session->make_iterator(RED_MIN_AREA);

    WaterfillObject object;
    while (iter->find_next(object, false)){
        if (object.aspect_ratio() < RED_MIN_ASPECT) continue;
        if ((double)object.width() / region.width() < RED_MIN_WIDTH_RATIO) continue;

        out_box = ImageFloatBox{
            m_scan_area.x + (double)object.min_x / region.width()  * m_scan_area.width,
            m_scan_area.y + (double)object.min_y / region.height() * m_scan_area.height,
            (double)object.width()  / region.width()  * m_scan_area.width,
            (double)object.height() / region.height() * m_scan_area.height
        };
        return true;
    }
    return false;
}

bool ShopItemHoverDetector::match_ocr(const ImageViewRGB32& screen, const ImageFloatBox& outline_box) const{
    m_last_name_box = ImageFloatBox{
        outline_box.x + NAME_X_OFFSET * outline_box.width,
        outline_box.y,
        NAME_X_WIDTH * outline_box.width,
        OCR_ROW_HEIGHT
    };
    const ImageFloatBox& name_box = m_last_name_box;

    const ShopItemNameReader& name_reader = ShopItemNameReader::instance();
    OCR::StringMatchResult name_result = name_reader.read_substring(
        m_logger, m_language, extract_box_reference(screen, name_box)
    );
    name_result.clear_beyond_log10p(ShopItemNameReader::MAX_LOG10P);
    if (name_result.results.empty()){
        return false;
    }
    if (name_result.results.begin()->second.token != m_item_slug){
        return false;
    }

    if (!m_ocr_logged){
        m_ocr_logged = true;
        name_result.log(m_logger, ShopItemNameReader::MAX_LOG10P, "want: " + m_item_slug);
    }
    return true;
}

bool ShopItemHoverDetector::detect(const ImageViewRGB32& screen){
    ImageFloatBox outline_box;
    if (!find_outline(screen, outline_box)){
        m_detected_box   = ImageFloatBox{0, 0, 0, 0};
        m_last_name_box  = ImageFloatBox{0, 0, 0, 0};
        m_ocr_logged     = false;
        return false;
    }
    if (!match_ocr(screen, outline_box)){
        m_last_name_box  = ImageFloatBox{0, 0, 0, 0};
        return false;
    }
    m_detected_box = outline_box;
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
    items.add(m_color, m_scan_area);
    if (m_detected_box.width > 0){
        items.add(m_color, m_detected_box);
    }
    if (m_last_name_box.width > 0){
        items.add(COLOR_BLUE, m_last_name_box);
    }
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
