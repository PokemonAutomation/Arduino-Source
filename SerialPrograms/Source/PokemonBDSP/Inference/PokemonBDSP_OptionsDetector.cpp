/*  Options Menu Detection and OCR
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include <map>
#include <vector>
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Logging/AbstractLogger.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/OCR/OCR_NumberReader.h"
#include "CommonTools/OCR/OCR_Routines.h"
#include "CommonTools/OCR/OCR_StringNormalization.h"
#include "PokemonBDSP_OptionsDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{

bool TutorialOptionsDetector::detect(const ImageViewRGB32& screen) const{
    ImageViewRGB32 panel = extract_box_reference(screen, ImageFloatBox{0.04, 0.065, 0.93, 0.70});
    size_t white = 0;
    for (size_t y = 0; y < panel.height(); y += 4){
        for (size_t x = 0; x < panel.width(); x += 4){
            uint32_t p = panel.pixel(x, y);
            if (((p >> 16) & 255) > 225 && ((p >> 8) & 255) > 225 && (p & 255) > 225){
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
    return samples > 0 && white / samples > 0.70 && total > 0 && blue > total / 4;
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
        {"textspeed", {"slow", "normal", "fast"}},
        {"changewindow", {"type1", "type2", "type3", "type4", "type5", "type6", "type7", "type8", "type9", "type10", "type11", "type12", "type13", "type14", "type15", "type16", "type17", "type18", "type19", "type20", "type21", "type22", "type23", "type24"}},
        {"battleeffects", {"on", "off"}},
        {"battlestyle", {"switch", "set"}},
        {"characters", {"hiragana", "kanji"}},
        {"sendtoboxes", {"manual", "automatic"}},
        {"givenicknames", {"give", "dontgive"}},
        {"gyroscope", {"use", "dontuse"}},
        {"autosave", {"on", "off"}},
        {"shownicknames", {"show", "dontshow"}},
        {"backgroundmusic", {"volume0", "volume1", "volume2", "volume3", "volume4", "volume5", "volume6", "volume7", "volume8", "volume9", "volume10"}},
        {"soundeffects", {"volume0", "volume1", "volume2", "volume3", "volume4", "volume5", "volume6", "volume7", "volume8", "volume9", "volume10"}},
        {"pokemoncries", {"volume0", "volume1", "volume2", "volume3", "volume4", "volume5", "volume6", "volume7", "volume8", "volume9", "volume10"}},
        {"verticalcamera", {"regular", "inverted"}},
        {"horizontalcamera", {"regular", "inverted"}},
        {"fossildiggingscreen", {"shake", "dontshake"}},
    };
    const auto setting = choices.find(row);
    if (!row.empty() && setting == choices.end()){
        return "";
    }
    const bool value = !row.empty();
    const bool number = row == "backgroundmusic" || row == "soundeffects" || row == "pokemoncries";
    // Retry unknown selected text with extra vertical padding for low-resolution glyphs.
    for (size_t attempt = 0; attempt < (value && !number ? 2u : 1u); attempt++){
        const double half_height = attempt == 0 ? 0.025 : 0.030;
        const ImageFloatBox box = !value ? ImageFloatBox{0.08, y - 0.025, 0.24, 0.05}
            : number ? ImageFloatBox{0.7728, y - 0.025, 0.0972, 0.05}
            : ImageFloatBox{0.33, y - half_height, 0.54, half_height * 2};
        const auto crop = extract_box_reference(screen, box);
        ImageRGB32 mask(crop.width(), crop.height());
        size_t ink_count = 0;
        size_t on = 0, off = 0;
        size_t left = crop.width(), right = 0;
        std::vector<size_t> column_ink(crop.width(), 0);
        for (size_t py = 0; py < crop.height(); py++){
            for (size_t px = 0; px < crop.width(); px++){
                const uint32_t p = crop.pixel(px, py);
                const int r = (p >> 16) & 255, g = (p >> 8) & 255, b = p & 255;
                // Isolate the selected value; other choices are still visible in black.
                const bool ink = value ? r > g + 30 && r > b + 30 : r + g + b < 500;
                mask.pixel(px, py) = ink ? 0xff000000 : 0xffffffff;
                if (ink){
                    ink_count++;
                    column_ink[px]++;
                    if (value && !number){
                        const double x = 0.33 + 0.54 * px / crop.width();
                        on += x >= 0.34 && x < 0.395;
                        off += x >= 0.63 && x < 0.685;
                    }
                }
            }
        }
        // Reuse the Autosave On/Off highlight positions. Short toggle labels are
        // unreliable OCR inputs; require one clear selection and reject competition.
        if (row == "autosave" || row == "battleeffects"){
            const size_t minimum = std::max(size_t(3), screen.width() * screen.height() / 207360);
            const std::string toggle = off >= minimum && on * 10 < off ? "off"
                : on >= minimum && off * 10 < on ? "on" : "";
            logger.log("[Options selection] " + row + ": " + (toggle.empty() ? "unknown" : toggle));
            return toggle;
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
        const auto text_image = mask.sub_image(left, 0, right - left, mask.height()).scale_to(
            std::max(size_t(1), (right - left) * 48 / mask.height()), 48);
        const int numeric = number ? OCR::read_number(logger, text_image) : -1;
        if (number && (numeric < 0 || numeric > 10)){
            return "";
        }
        const std::string text = number ? std::to_string(numeric) : OCR::ocr_read(language, text_image);
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
            const auto& translations = dictionary.to_object_throw().get_object_throw(candidate);
            const auto& translation = translations.get_value_throw(language_data(language).code);
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
