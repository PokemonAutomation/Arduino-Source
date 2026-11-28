/*  Options Menu Detection and OCR
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include <array>
#include <cmath>
#include <map>
#include <vector>
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Logging/AbstractLogger.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/Images/ImageManip.h"
#include "CommonTools/OCR/OCR_Routines.h"
#include "CommonTools/OCR/OCR_StringNormalization.h"
#include "PokemonBDSP_ButtonDetector.h"
#include "PokemonBDSP_OptionsDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{

bool TutorialOptionsDetector::detect(const ImageViewRGB32& screen) const{
    if (!screen){
        return false;
    }
    ImageViewRGB32 panel = extract_box_reference(screen, ImageFloatBox{0.04, 0.065, 0.93, 0.70});
    size_t white = 0, light_neutral = 0;
    for (size_t y = 0; y < panel.height(); y += 4){
        for (size_t x = 0; x < panel.width(); x += 4){
            uint32_t p = panel.pixel(x, y);
            const int r = (p >> 16) & 255, g = (p >> 8) & 255, b = p & 255;
            const int low = std::min({r, g, b});
            light_neutral += low >= 145 && std::max({r, g, b}) - low < 35;
            if (low > 225){
                white++;
            }
        }
    }
    double samples = (double)(((panel.width() + 3) / 4) * ((panel.height() + 3) / 4));
    // Distinguish Options from the icon menu and bag using its
    // blue heading and full-width white settings panel.
    ImageViewRGB32 heading = extract_box_reference(screen, ImageFloatBox{0.01, 0.005, 0.20, 0.025});
    size_t blue = 0, total = 0;
    for (size_t y = 0; y < heading.height(); y++){
        for (size_t x = 0; x < heading.width(); x++){
            uint32_t p = heading.pixel(x, y);
            int r = (p >> 16) & 255, g = (p >> 8) & 255, b = p & 255;
            blue += b > r + 20 && g > r + 10;
            total++;
        }
    }
    if (samples == 0 || total == 0){
        return false;
    }
    if (blue > total / 4 && (white / samples > 0.70 || (light_neutral / samples > 0.70 &&
        TutorialOptionPositionDetector().selected_row(screen) >= 0))){
        return true;
    }
    // Like PLZA's map detector, combine several button letters with screen-specific
    // structure. Footer labels vary by language, so search for ordered A/Y/B icons.
    if (light_neutral / samples <= 0.60 || TutorialOptionPositionDetector().selected_row(screen) < 0){
        return false;
    }
    static const ImageFloatBox footer{0.40, 0.955, 0.60, 0.045};
    static const ButtonDetector confirm(ButtonType::A, footer);
    static const ButtonDetector reset(ButtonType::Y, footer);
    static const ButtonDetector exit(ButtonType::B, footer);
    ImageFloatBox a, y, b;
    if (!confirm.detect(screen, &a) || !reset.detect(screen, &y) || !exit.detect(screen, &b)){
        return false;
    }
    const double ay = a.y + a.height * 0.5;
    const double yy = y.y + y.height * 0.5;
    const double by = b.y + b.height * 0.5;
    return a.x + a.width < y.x && y.x + y.width < b.x &&
        std::abs(ay - yy) < 0.01 && std::abs(yy - by) < 0.01;
}

// Locate the two long red horizontal lines surrounding the
// selected row. Scrolling changes their position automatically.
double TutorialOptionPositionDetector::selected_row(const ImageViewRGB32& screen) const{
    int first = -1, last = -1;
    for (size_t y = (size_t)(screen.height() * 0.08); y < screen.height() * 0.75; y++){
        size_t red = 0, total = 0;
        for (size_t x = (size_t)(screen.width() * 0.08); x < screen.width() * 0.90; x += 2){
            uint32_t p = screen.pixel(x, y);
            int r = (p >> 16) & 255, g = (p >> 8) & 255, b = p & 255;
            red += r > 150 && r > g + 40 && r > b + 40;
            total++;
        }
        if (total && red > total * 0.70){
            if (first < 0){
                first = (int)y;
            }
            last = (int)y;
        }
    }
    if (first < 0 || last - first < screen.height() * 0.04 || last - first > screen.height() * 0.12){
        return -1;
    }
    return (first + last) / (2.0 * screen.height());
}
// Locate selected red value text separately from the row border.
bool TutorialOptionPositionDetector::selected_value(const ImageViewRGB32& screen, double y) const{
    ImageViewRGB32 value = extract_box_reference(screen, ImageFloatBox{0.33, y - 0.025, 0.54, 0.05});
    size_t red = 0;
    for (size_t py = 0; py < value.height(); py++){
        for (size_t px = 0; px < value.width(); px++){
            uint32_t p = value.pixel(px, py);
            int r = (p >> 16) & 255, g = (p >> 8) & 255, b = p & 255;
            red += r > g + 20 && r > b + 20;
        }
    }
    return red > value.width() * value.height() * 0.0005;
}

std::string TutorialOptionPositionDetector::read_text(Logger& logger, Language language,
    const ImageViewRGB32& screen, double y, const std::string& row) const{
    if (!screen || y < 0.08 || y > 0.75 || language == Language::None){
        return "";
    }
    static const std::map<std::string, std::vector<std::string>> choices{
        {"text-speed", {"slow", "normal", "fast"}},
        {"change-window", {"type-1", "type-2", "type-3", "type-4", "type-5", "type-6", "type-7", "type-8", "type-9", "type-10", "type-11", "type-12", "type-13", "type-14", "type-15", "type-16", "type-17", "type-18", "type-19", "type-20", "type-21", "type-22", "type-23", "type-24"}},
        {"battle-effects", {"on", "off"}},
        {"battle-style", {"switch", "set"}},
        {"characters", {"hiragana", "kanji"}},
        {"send-to-boxes", {"manual", "automatic"}},
        {"give-nicknames", {"give", "dont-give"}},
        {"gyroscope", {"use", "dont-use"}},
        {"autosave", {"on", "off"}},
        {"show-nicknames", {"show", "dont-show"}},
        {"vertical-camera", {"regular", "inverted"}},
        {"horizontal-camera", {"regular", "inverted"}},
        {"fossil-digging-screen", {"shake", "dont-shake"}},
    };
    const auto setting = choices.find(row);
    if (!row.empty() && setting == choices.end()){
        return "";
    }
    const bool value = !row.empty();
    // Retry row labels with the original antialiased strokes. Window styles
    // retain the wider second crop for the selected red text.
    const size_t attempts = !value ? 3 : 2;
    for (size_t attempt = 0; attempt < attempts; attempt++){
        const double half_height = attempt + 1 == attempts ? 0.030 : 0.025;
        const ImageFloatBox box = !value ? ImageFloatBox{0.08, y - half_height, 0.246, half_height * 2}
            : ImageFloatBox{0.33, y - half_height, 0.54, half_height * 2};
        const auto crop = extract_box_reference(screen, box);
        ImageRGB32 mask(crop.width(), crop.height());
        size_t ink_count = 0;
        // Count ink near the beginning of each fixed choice; the positions
        // are independent of the length of its translated label.
        const size_t slots = value && setting->second.size() <= 3 ? setting->second.size() : 0;
        const std::array<double, 3> starts{0.34, slots == 2 ? 0.635 : 0.525, 0.708};
        std::array<size_t, 3> highlights{};
        const auto background = crop.pixel(0, 0);
        const int backdrop = ((background >> 16) & 255) + ((background >> 8) & 255) + (background & 255);
        size_t left = crop.width(), right = 0;
        std::vector<size_t> column_ink(crop.width(), 0);
        for (size_t py = 0; py < crop.height(); py++){
            for (size_t px = 0; px < crop.width(); px++){
                const uint32_t p = crop.pixel(px, py);
                const int r = (p >> 16) & 255, g = (p >> 8) & 255, b = p & 255;
                // Isolate the selected value; other choices are still visible in black.
                const bool ink = value ? r > g + 30 && r > b + 30 : r + g + b < std::min(500, backdrop - 120);
                mask.pixel(px, py) = ink ? 0xff000000 : 0xffffffff;
                if (ink){
                    ink_count++;
                    column_ink[px]++;
                    for (size_t i = 0; i < slots; i++){
                        const double x = 0.33 + 0.54 * px / crop.width();
                        highlights[i] += x >= starts[i] && x < starts[i] + 0.045;
                    }
                }
            }
        }
        if (slots != 0){
            const auto best = std::max_element(highlights.begin(), highlights.end());
            const size_t index = best - highlights.begin();
            const size_t ink = *best;
            *best = 0;
            const size_t minimum = std::max(size_t(3), screen.width() * screen.height() / 207360);
            if (ink < minimum || *std::max_element(highlights.begin(), highlights.end()) * 10 >= ink){
                return "";
            }
            return setting->second[index];
        }
        // Ignore isolated colored compression pixels when locating the text span.
        for (size_t px = 0; px < crop.width(); px++){
            if (column_ink[px] >= std::max(size_t(2), crop.height() / 12)){
                left = std::min(left, px);
                right = std::max(right, px + 1);
            }
        }
        if (ink_count < 3 || left >= right){
            continue;
        }
        left = left > 8 ? left - 8 : 0;
        right = std::min(right + 8, crop.width());
        const bool original_label = !value && attempt == 1;
        const auto region = original_label ? crop.sub_image(left, 0, right - left, crop.height())
            : mask.sub_image(left, 0, right - left, mask.height());
        auto text_image = region.scale_to(std::max(size_t(1), region.width() * 48 / region.height()), 48);
        if (original_label){
            text_image = pad_image(text_image, 8, 0xffffffff);
        }
        const std::string text = OCR::ocr_read(language, text_image);
        const auto normalized = OCR::normalize_utf32(text);
        if (normalized.empty()){
            continue;
        }
        static const JsonValue dictionary = load_json_file(RESOURCE_PATH() + "PokemonBDSP/Settings.json");
        std::vector<std::string> candidates;
        if (value){
            candidates = setting->second;
        }else{
            for (const auto& item : choices){
                candidates.push_back(item.first);
            }
        }
        std::string token;
        for (const auto& candidate : candidates){
            const auto& translations = dictionary.to_object_throw().get_object_throw(language_data(language).code);
            const auto& translation = translations.get_value_throw(candidate);
            bool matched = false;
            if (translation.is_array()){
                for (const auto& spelling : translation.to_array_throw()){
                    matched |= normalized == OCR::normalize_utf32(spelling.to_string_throw());
                }
            }else{
                matched = normalized == OCR::normalize_utf32(translation.to_string_throw());
            }
            if (matched){
                if (!token.empty()){
                    return "";
                }
                token = candidate;
            }
        }
        logger.log("[Options OCR] " + (value ? row : "row") + ": \"" + text + "\" -> " +
            (token.empty() ? "unknown" : token));
        if (!token.empty()){
            return token;
        }
    }
    return "";
}

}
}
}
