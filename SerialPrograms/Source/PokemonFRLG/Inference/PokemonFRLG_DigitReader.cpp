/*  FRLG Digit Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include <array>
#include <map>
#include <memory>
#include <string>
#include <opencv2/imgproc.hpp>
#include "Common/Cpp/Color.h" // needed for COLOR_RED, COLOR_ORANGE
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Logging/AbstractLogger.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTypes/ImageRGB32_OpenCV.h"
#include "CommonTools/ImageMatch/ExactImageMatcher.h"
#include "CommonTools/Images/BinaryImage_FilterRgb32.h"

#include "PokemonFRLG_DigitReader.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

// ---------------------------------------------------------------------------
// Template store: loads 10 digit matchers from a resource sub-directory.
// Results are cached in a static map keyed by template type.
// Supports:
// - StatBox (yellow stat boxes): PokemonFRLG/Digits/
// - LevelBox (lilac level box): PokemonFRLG/LevelDigits/
// - DialogBox (white dialog box): PokemonFRLG/DialogDigits/
// ---------------------------------------------------------------------------

static std::string get_template_path(DigitTemplateType type){
    switch (type){
    case DigitTemplateType::StatBox:
        return "PokemonFRLG/Digits/";
    case DigitTemplateType::LevelBox:
        return "PokemonFRLG/LevelDigits/";
    case DigitTemplateType::DialogBox:
        return "PokemonFRLG/DialogDigits/";
    default:
        return "PokemonFRLG/Digits/";
    }
}

struct DigitTemplates{
    // matchers[d] is the matcher for digit d (0-9), or nullptr if missing.
    std::array<std::unique_ptr<ImageMatch::ExactImageMatcher>, 10> matchers;
    bool any_loaded = false;

    explicit DigitTemplates(DigitTemplateType template_type){
        std::string resource_subdir = get_template_path(template_type);
        for (int d = 0; d < 10; ++d){
            std::string path =
                RESOURCE_PATH() + resource_subdir + std::to_string(d) + ".png";
            try{
                ImageRGB32 img(path);
                if (img.width() > 0){
                    matchers[d] =
                            std::make_unique<ImageMatch::ExactImageMatcher>(std::move(img));
                    any_loaded = true;
                }
            }catch (...){
                // Template image missing - slot stays nullptr.
            }
        }
        if (!any_loaded){
            throw FileException(
                nullptr, PA_CURRENT_FUNCTION,
                "Failed to load any digit templates", resource_subdir
            );
        }
    }

    static const DigitTemplates& get(DigitTemplateType template_type){
        static std::map<DigitTemplateType, DigitTemplates> cache;
        auto it = cache.find(template_type);
        if (it == cache.end()){
            it = cache.emplace(template_type, DigitTemplates(template_type)).first;
        }
        return it->second;
    }
};

// Shrink a digit's bounding box down to the glyph itself.
static ImagePixelBox tighten_to_glyph(const ImageViewRGB32& region, const ImagePixelBox& box){
    const size_t PADDING = 1;

    auto brightness = [&](size_t x, size_t y) -> uint32_t{
        uint32_t p = region.pixel(x, y);
        return (p & 0xff) + ((p >> 8) & 0xff) + ((p >> 16) & 0xff);
    };

    uint32_t min_brightness = 0xffffffff;
    uint32_t max_brightness = 0;
    for (size_t y = box.min_y; y < box.max_y; ++y){
        for (size_t x = box.min_x; x < box.max_x; ++x){
            uint32_t b = brightness(x, y);
            min_brightness = std::min(min_brightness, b);
            max_brightness = std::max(max_brightness, b);
        }
    }
    if (min_brightness > max_brightness){
        return box;     //  Empty box.
    }
    uint32_t glyph_max = (min_brightness + max_brightness) / 2;

    size_t glyph_min_x = box.max_x;
    size_t glyph_min_y = box.max_y;
    size_t glyph_max_x = box.min_x;
    size_t glyph_max_y = box.min_y;
    for (size_t y = box.min_y; y < box.max_y; ++y){
        for (size_t x = box.min_x; x < box.max_x; ++x){
            if (brightness(x, y) > glyph_max){
                continue;
            }
            glyph_min_x = std::min(glyph_min_x, x);
            glyph_min_y = std::min(glyph_min_y, y);
            glyph_max_x = std::max(glyph_max_x, x);
            glyph_max_y = std::max(glyph_max_y, y);
        }
    }
    if (glyph_min_x > glyph_max_x || glyph_min_y > glyph_max_y){
        return box;     //  Nothing dark enough to be a glyph.
    }

    //  max_x/max_y are exclusive, so the last glyph pixel needs a +1 of its own.
    return ImagePixelBox(
        glyph_min_x >= box.min_x + PADDING ? glyph_min_x - PADDING : box.min_x,
        glyph_min_y >= box.min_y + PADDING ? glyph_min_y - PADDING : box.min_y,
        std::min(glyph_max_x + 1 + PADDING, box.max_x),
        std::min(glyph_max_y + 1 + PADDING, box.max_y)
    );
}

// ---------------------------------------------------------------------------
// Main function
// ---------------------------------------------------------------------------
int read_digits_waterfill_template(
    Logger& logger,
    const ImageViewRGB32& stat_region,
    DigitTemplateType template_type,
    double rmsd_threshold,
    const std::string& dump_prefix,
    uint8_t binarize_high,
    bool save_debug_images
){
    using namespace Kernels::Waterfill;

    if (!stat_region){
        logger.log("DigitReader: empty stat region.", COLOR_RED);
        return -1;
    }

    // ------------------------------------------------------------------
    // Step 1: Gaussian blur on the NATIVE resolution image.
    //   The GBA pixel font has 1-pixel gaps between segments.
    //   A 5x5 kernel applied twice bridges those gaps so that waterfill
    //   sees each digit as a single connected component.
    // ------------------------------------------------------------------
    cv::Mat src = to_OpenCV_ref(stat_region);
    cv::Mat blurred;
    src.copyTo(blurred);
    cv::GaussianBlur(blurred, blurred, cv::Size(5, 5), 1.5);
    cv::GaussianBlur(blurred, blurred, cv::Size(5, 5), 1.5);

    ImageRGB32 blurred_img(blurred.cols, blurred.rows);
    blurred.copyTo(to_OpenCV_ref(blurred_img));

    // ------------------------------------------------------------------
    // Step 2: Binarise the blurred image.
    //   Pixels where ALL channels <= binarize_high become 1 (foreground).
    //   Default 0xBE (190) works for yellow stat boxes.
    //   Use 0x7F (127) for the lilac level box to prevent the blurred
    //   lilac background (B~208, drops to ~156 near shadows) from being
    //   captured and merging digit blobs.
    // ------------------------------------------------------------------
    uint32_t bh = binarize_high;
    uint32_t binarize_color = 0xff000000u | (bh << 16) | (bh << 8) | bh;
    PackedBinaryMatrix matrix =
        compress_rgb32_to_binary_range(blurred_img, 0xff000000u, binarize_color);

    // ------------------------------------------------------------------
    // Step 3: Waterfill - find connected dark blobs (individual digits).
    //   Minimum area of 4 pixels to discard lone noise specks.
    //   Sort blobs left-to-right by their left edge (min_x).
    // ------------------------------------------------------------------
    const size_t min_area = 4;
    std::map<size_t, WaterfillObject> blobs; // key = min_x, auto-sorted L->R
    {
        std::unique_ptr<WaterfillSession> session = make_WaterfillSession(matrix);
        auto iter = session->make_iterator(min_area);
        WaterfillObject obj;
        while (blobs.size() < 8 && iter->find_next(obj, false)){
            // Require at least 5px wide AND 15px tall to discard noise fragments.
            if (obj.max_x - obj.min_x < 5 || obj.max_y - obj.min_y < 15){
                continue;
            }
            // Use min_x as key so the map is automatically sorted left-to-right.
            // If two blobs share an identical min_x, bump the key slightly.
            size_t key = obj.min_x;
            while (blobs.count(key)){
                ++key;
            }
            blobs.emplace(key, std::move(obj));
        }
    }

    if (blobs.empty()){
        logger.log("DigitReader: waterfill found no digit blobs.", COLOR_RED);
        return -1;
    }

    // ------------------------------------------------------------------
    // Step 4: For each blob, crop the UNBLURRED original stat_region to
    //   the blob's bounding box, then template-match against all 10 digit
    //   templates using ExactImageMatcher::rmsd(). Pick the lowest RMSD.
    // ------------------------------------------------------------------
    const DigitTemplates& templates = DigitTemplates::get(template_type);
    std::string result_str;

    for (const auto &kv : blobs){
        const WaterfillObject &obj = kv.second;

        size_t width = obj.max_x - obj.min_x;
        size_t height = obj.max_y - obj.min_y;

        int expected_digits = 1;
        // GBA font digits are typically narrower than they are tall (aspect ~0.6).
        // If the blob's width is wider than expected for a single digit, it's a
        // merged blob.
        if (width > height * 1.5){
            expected_digits = 3; // e.g. "100"
        }else if (width > height * 0.8){
            expected_digits = 2; // e.g. "23"
        }

        size_t split_w = width / expected_digits;

        for (int i = 0; i < expected_digits; ++i){
            size_t min_x = obj.min_x + i * split_w;
            size_t max_x = (i == expected_digits - 1) ? obj.max_x : obj.min_x + (i + 1) * split_w;

            // Crop the unblurred region to the split bounding box,
            // shrink to the glyph so a captured drop shadow doesn't cause an offset
            ImagePixelBox bbox = tighten_to_glyph(
                stat_region, ImagePixelBox(min_x, obj.min_y, max_x, obj.max_y)
            );
            ImageViewRGB32 crop = extract_box_reference(stat_region, bbox);

            if (save_debug_images && dump_prefix == "levelDigit"){
                crop.save("DebugDumps/" + dump_prefix + "_x" + std::to_string(min_x) + "_split_raw.png");
            }

            // Compute RMSD against each digit template; pick the minimum.
            // If no templates are loaded (extraction mode), skip matching entirely.
            double best_rmsd = 9999.0;
            int best_digit = -1;
            if (templates.any_loaded){
                for (int d = 0; d < 10; ++d){
                    if (!templates.matchers[d])
                        continue;
                    double r = templates.matchers[d]->rmsd(crop);
                    if (r < best_rmsd){
                        best_rmsd = r;
                        best_digit = d;
                    }
                }
            }

            if (best_rmsd > rmsd_threshold){
                // Always save the raw crop for user inspection / template extraction.
                if (save_debug_images){
                    crop.save(
                        "DebugDumps/" + dump_prefix + "_x" + std::to_string(min_x) + "_raw.png"
                    );
                }
                logger.log(
                    "DigitReader: blob at x=" + std::to_string(min_x) +
                    " skipped (best RMSD=" + std::to_string(best_rmsd) +
                    ", threshold=" + std::to_string(rmsd_threshold) + ").",
                    COLOR_ORANGE
                );
                continue;
            }

            logger.log(
                "DigitReader: blob at x=" + std::to_string(min_x) +
                " -> digit " + std::to_string(best_digit) +
                " (RMSD=" + std::to_string(best_rmsd) + ")"
            );
            // Save crop with prefix so level and stat crops are distinguishable.
            if (save_debug_images){
                crop.save(
                    "DebugDumps/" + dump_prefix + "_x" + std::to_string(min_x) +
                    "_match" + std::to_string(best_digit) + ".png"
                );
            }
            result_str += static_cast<char>('0' + best_digit);
        }
    }

    if (result_str.empty()){
        return -1;
    }

    int number = std::atoi(result_str.c_str());
    logger.log(
        "DigitReader: \"" + result_str + "\" -> " +
        std::to_string(number)
    );
    return number;
}

} // namespace PokemonFRLG
} // namespace NintendoSwitch
} // namespace PokemonAutomation

