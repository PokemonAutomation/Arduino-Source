/*  FRLG Digit Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <map>
#include <string>
#include <vector>
#include <memory>
#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/Images/BinaryImage_FilterRgb32.h"
#include "CommonTools/ImageMatch/ExactImageMatcher.h"
#include "Common/Cpp/Logging/AbstractLogger.h"
#include "Common/Cpp/Color.h"  // needed for COLOR_RED, COLOR_ORANGE
#include "PokemonFRLG_DigitReader.h"

#include <opencv2/imgproc.hpp>

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonFRLG {


// ---------------------------------------------------------------------------
// Template store: loads 10 digit matchers from a resource sub-directory.
// Results are cached in a static map keyed by subdirectory name so both
// "PokemonFRLG/Digits/" (yellow stat boxes) and
// "PokemonFRLG/LevelDigits/" (lilac level box) can co-exist.
// ---------------------------------------------------------------------------
struct DigitTemplates {
    // matchers[d] is the matcher for digit d (0-9), or nullptr if missing.
    std::vector<std::unique_ptr<ImageMatch::ExactImageMatcher>> matchers;
    bool any_loaded = false;

    explicit DigitTemplates(const std::string& resource_subdir) {
        matchers.resize(10);
        for (int d = 0; d < 10; ++d) {
            std::string path = RESOURCE_PATH() + resource_subdir +
                               std::to_string(d) + ".png";
            try {
                ImageRGB32 img(path);
                if (img.width() > 0) {
                    matchers[d] = std::make_unique<ImageMatch::ExactImageMatcher>(std::move(img));
                    any_loaded  = true;
                }
            } catch (...) {
                // Template image missing - slot stays nullptr.
            }
        }
    }

    static const DigitTemplates& get(const std::string& resource_subdir) {
        static std::map<std::string, DigitTemplates> cache;
        auto it = cache.find(resource_subdir);
        if (it == cache.end()) {
            it = cache.emplace(resource_subdir,
                               DigitTemplates(resource_subdir)).first;
        }
        return it->second;
    }
};


// ---------------------------------------------------------------------------
// Main function
// ---------------------------------------------------------------------------
int read_digits_waterfill_template(
    Logger& logger,
    const ImageViewRGB32& stat_region,
    double rmsd_threshold,
    const std::string& template_subdir,
    const std::string& dump_prefix,
    uint8_t binarize_high
) {
    using namespace Kernels::Waterfill;

    if (!stat_region) {
        logger.log("DigitReader: empty stat region.", COLOR_RED);
        return -1;
    }

    // ------------------------------------------------------------------
    // Step 1: Gaussian blur on the NATIVE resolution image.
    //   The GBA pixel font has 1-pixel gaps between segments.
    //   A 5x5 kernel applied twice bridges those gaps so that waterfill
    //   sees each digit as a single connected component.
    // ------------------------------------------------------------------
    cv::Mat src = stat_region.to_opencv_Mat();
    cv::Mat blurred;
    src.copyTo(blurred);
    cv::GaussianBlur(blurred, blurred, cv::Size(5, 5), 1.5);
    cv::GaussianBlur(blurred, blurred, cv::Size(5, 5), 1.5);

    ImageRGB32 blurred_img(blurred.cols, blurred.rows);
    blurred.copyTo(blurred_img.to_opencv_Mat());

    // ------------------------------------------------------------------
    // Step 2: Binarise the blurred image.
    //   Pixels where ALL channels <= binarize_high become 1 (foreground).
    //   Default 0xBE (190) works for yellow stat boxes.
    //   Use 0x7F (127) for the lilac level box to prevent the blurred
    //   lilac background (B≈208, drops to ~156 near shadows) from being
    //   captured and merging digit blobs.
    // ------------------------------------------------------------------
    uint32_t bh = binarize_high;
    uint32_t binarize_color = 0xff000000u | (bh << 16) | (bh << 8) | bh;
    PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(
        blurred_img,
        0xff000000u, binarize_color
    );

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
        while (blobs.size() < 8 && iter->find_next(obj, false)) {
            // Require at least 3px wide AND 3px tall to discard noise fragments.
            if (obj.max_x - obj.min_x < 3 || obj.max_y - obj.min_y < 3) continue;
            // Use min_x as key so the map is automatically sorted left-to-right.
            // If two blobs share an identical min_x, bump the key slightly.
            size_t key = obj.min_x;
            while (blobs.count(key)) ++key;
            blobs.emplace(key, std::move(obj));
        }
    }

    if (blobs.empty()) {
        logger.log("DigitReader: waterfill found no digit blobs.", COLOR_RED);
        return -1;
    }

    // ------------------------------------------------------------------
    // Step 4: For each blob, crop the UNBLURRED original stat_region to
    //   the blob's bounding box, then template-match against all 10 digit
    //   templates using ExactImageMatcher::rmsd(). Pick the lowest RMSD.
    // ------------------------------------------------------------------
    const DigitTemplates& templates = DigitTemplates::get(template_subdir);
    std::string result_str;

    for (const auto& kv : blobs) {
        const WaterfillObject& obj = kv.second;

        // Crop original (unblurred) region to the blob bounding box.
        ImagePixelBox bbox(obj.min_x, obj.min_y, obj.max_x, obj.max_y);
        ImageViewRGB32 crop = extract_box_reference(stat_region, bbox);

        // Compute RMSD against each digit template; pick the minimum.
        // If no templates are loaded (extraction mode), skip matching entirely.
        double best_rmsd  = 9999.0;
        int    best_digit = -1;
        if (templates.any_loaded) {
            for (int d = 0; d < 10; ++d) {
                if (!templates.matchers[d]) continue;
                double r = templates.matchers[d]->rmsd(crop);
                if (r < best_rmsd) {
                    best_rmsd  = r;
                    best_digit = d;
                }
            }
        }

        if (best_rmsd > rmsd_threshold) {
            // Always save the crop for user inspection / template extraction.
            crop.save("DebugDumps/" + dump_prefix + "_x" + std::to_string(obj.min_x) + ".png");
            logger.log(
                "DigitReader: blob at x=" + std::to_string(obj.min_x) +
                " skipped (best RMSD=" +
                std::to_string(best_rmsd) + ", threshold=" +
                std::to_string(rmsd_threshold) + ").",
                COLOR_ORANGE
            );
            continue;
        }

        logger.log(
            "DigitReader: blob at x=" + std::to_string(obj.min_x) +
            " -> digit " + std::to_string(best_digit) +
            " (RMSD=" + std::to_string(best_rmsd) + ")"
        );
        // Save crop with prefix so level and stat crops are distinguishable.
        crop.save("DebugDumps/" + dump_prefix + "_x" + std::to_string(obj.min_x) +
                  "_match" + std::to_string(best_digit) + ".png");
        result_str += static_cast<char>('0' + best_digit);
    }

    if (result_str.empty()) {
        return -1;
    }

    int number = std::atoi(result_str.c_str());
    logger.log("DigitReader: \"" + result_str + "\" -> " + std::to_string(number));
    return number;
}


} // namespace PokemonFRLG
} // namespace NintendoSwitch
} // namespace PokemonAutomation
