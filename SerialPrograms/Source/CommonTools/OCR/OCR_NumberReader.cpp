/*  OCR Number Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include <map>
#include "Common/Cpp/Unicode.h"
#include "Common/Cpp/Logging/AbstractLogger.h"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "CommonFramework/Language.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Tools/GlobalThreadPools.h"
#include "CommonTools/Images/ImageManip.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonTools/Images/BinaryImage_FilterRgb32.h"
#include "OCR_RawOCR.h"
#include "OCR_NumberReader.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace OCR{


std::string run_number_normalization(const std::string& input){
    static const std::map<char32_t, char> SUBSTITUTION_TABLE{
        {'0', '0'},
        {'1', '1'},
        {'2', '2'},
        {'3', '3'},
        {'4', '4'},
        {'5', '5'},
        {'6', '6'},
        {'7', '7'},
        {'8', '8'},
        {'9', '9'},

        //  Common misreads.
        {'|', '1'},
        {']', '1'},
        {'[', '6'},
        {'l', '1'},
        {'i', '1'},
        {'A', '4'},
        {'a', '4'},
        {'S', '5'},
        {'s', '5'},
        {'/', '7'},
        {'g', '9'},

        //  Japanese OCR likes to do this.
        {U'🄋', '0'},
        {U'①', '1'},
        {U'②', '2'},
        {U'③', '3'},
        {U'④', '4'},
        {U'⑤', '5'},
        {U'⑥', '6'},
        {U'⑦', '7'},
        {U'⑧', '8'},
        {U'⑨', '9'},
    };

    std::string normalized;
    for (char32_t ch : utf8_to_utf32(input)){
        auto iter = SUBSTITUTION_TABLE.find(ch);
        if (iter == SUBSTITUTION_TABLE.end()){
            continue;
        }
        normalized += iter->second;
    }

    return normalized;
}


int read_number(Logger& logger, const ImageViewRGB32& image, Language language){
    std::string ocr_text = OCR::ocr_read(language, image, OCR::PageSegMode::SINGLE_LINE);
    std::string normalized = run_number_normalization(ocr_text);

    std::string str;
    for (char ch : ocr_text){
        if (ch != '\r' && ch != '\n'){
            str += ch;
        }
    }

    if (normalized.empty()){
        logger.log("OCR Text: \"" + str + "\" -> \"" + normalized + "\" -> Unable to read.", COLOR_RED);
        return -1;
    }

    int number = std::atoi(normalized.c_str());
    logger.log("OCR Text: \"" + str + "\" -> \"" + normalized + "\" -> " + std::to_string(number));

    return number;
}


// Run OCR on each individual character in the string of numbers.
// Return empty string if OCR fails.
//
// - text_inside_range: binary filter is applied to the image so that any pixels within
//   the color range will be turned black, and everything else will be white
// - width_max: return empty string if any character's width is greater than width_max
//   (likely means that two characters are touching, and so are treated as one large
//   character)
// - min_digit_area: if a character has area (aka pixel count) smaller than this value
//   (likely noise or punctuations), skip this character
// - check_empty_string: if set to true, return empty string (and stop evaluation) if
//   any character returns an empty string from OCR
std::string read_number_waterfill_no_normalization(
    Logger& logger, const ImageViewRGB32& image,
    uint32_t rgb32_min, uint32_t rgb32_max,    
    bool text_inside_range = true,
    size_t width_max = (size_t)-1,
    size_t min_digit_area = 20,
    bool check_empty_string = false
){
    using namespace Kernels::Waterfill;

    //  Direct OCR is unreliable. Instead, we will waterfill each character
    //  to isolate them, then OCR them individually.

    ImageRGB32 filtered = to_blackwhite_rgb32_range(
        image,
        text_inside_range,
        rgb32_min, rgb32_max
    );

//    static int c = 0;
//    filtered.save(std::format("zztest-{:#x}-{:#x}-{}.png", rgb32_min, rgb32_max, c++));

    PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(filtered, 0xff000000, 0xff7f7f7f);

    std::map<size_t, WaterfillObject> map;
    {
        std::unique_ptr<WaterfillSession> session = make_WaterfillSession(matrix);
        auto iter = session->make_iterator(min_digit_area);
        WaterfillObject object;
        while (map.size() < 16 && iter->find_next(object, true)){
            if (object.width() > width_max){
                logger.log("OCR fail: one of characters exceeded max width.", COLOR_RED);
                return "";
            }
            map.emplace(object.min_x, std::move(object));
        }
    }

    std::string ocr_text;
    for (const auto& item : map){
        const WaterfillObject& object = item.second;
        ImageRGB32 cropped = extract_box_reference(filtered, object).copy();
        PackedBinaryMatrix tmp(object.packed_matrix());
        filter_by_mask(tmp, cropped, Color(0xffffffff), true);

        //  Tesseract doesn't like numbers that are too big. So scale it down.
//        cout << "height = " << cropped.height() << endl;
        if (cropped.height() > 60){
            cropped = cropped.scale_to(cropped.width() * 60 / cropped.height(), 60);
        }

        ImageRGB32 padded = pad_image(cropped, 1 * cropped.width(), 0xffffffff);
        std::string ocr = OCR::ocr_read(Language::English, padded, OCR::PageSegMode::SINGLE_CHAR);

//        padded.save("zztest-cropped" + std::to_string(c) + "-" + std::to_string(i++) + ".png");
        // std::cout << ocr[0] << std::endl;
        if (!ocr.empty()){
            ocr_text += ocr[0];
        }else if (check_empty_string){
            logger.log("OCR fail: one of characters read as empty string.", COLOR_RED);
            // static int i = 0;
            // padded.save("zztest-cropped" + std::to_string(c) + "-" + std::to_string(i++) + ".png");
            return "";
        }
    }

    return ocr_text;

}

bool is_digits(const std::string &str)
{
    return std::all_of(str.begin(), str.end(), ::isdigit);
}


int read_number_waterfill(
    Logger& logger, const ImageViewRGB32& image,
    uint32_t rgb32_min, uint32_t rgb32_max,    
    bool text_inside_range,
    int8_t line_index
){
    std::string ocr_text = read_number_waterfill_no_normalization(
        logger,
        image,
        rgb32_min, rgb32_max,
        text_inside_range
    );

    std::string normalized = run_number_normalization(ocr_text);

    std::string line_index_str = "";
    if (line_index != -1){
        line_index_str = "Line " + std::to_string(line_index) + ": ";
    }
    if (normalized.empty()){
        logger.log(line_index_str + "OCR Text: \"" + ocr_text + "\" -> \"" + normalized + "\" -> Unable to read.", COLOR_RED);
        return -1;
    }

    int number = std::atoi(normalized.c_str());
    logger.log(line_index_str + "OCR Text: \"" + ocr_text + "\" -> \"" + normalized + "\" -> " + std::to_string(number));

    return number;
}


int read_number_waterfill_multifilter(
    Logger& logger, const ImageViewRGB32& image,
    std::vector<std::pair<uint32_t, uint32_t>> filters,    
    bool text_inside_range,
    bool prioritize_numeric_only_results, 
    size_t width_max,
    size_t min_digit_area,
    int8_t line_index
){
    std::string line_index_str = "";
    if (line_index != -1){
        line_index_str = "Line " + std::to_string(line_index) + ": ";
    }

    SpinLock lock;
    std::map<int, uint8_t> candidates;
    GlobalThreadPools::normal_inference().run_in_parallel(
        [&](size_t index){
            std::pair<uint32_t, uint32_t> filter = filters[index];

            uint32_t rgb32_min = filter.first;
            uint32_t rgb32_max = filter.second;
            bool check_empty_string = true;
            std::string ocr_text = read_number_waterfill_no_normalization(
                logger,
                image,
                rgb32_min, rgb32_max,
                text_inside_range,
                width_max,
                min_digit_area,
                check_empty_string
            );

            std::string normalized = run_number_normalization(ocr_text);
            if (normalized.empty()){
                // logger.log("OCR Text: \"" + ocr_text + "\" -> \"" + normalized + "\" -> Unable to read.", COLOR_RED);
                return;
            }

            int candidate = std::atoi(normalized.c_str());
            logger.log(line_index_str + "OCR Text: \"" + ocr_text + "\" -> \"" + normalized + "\" -> " + std::to_string(candidate));

            uint8_t weight = prioritize_numeric_only_results && is_digits(ocr_text)
                ? 2
                : 1;

            WriteSpinLock lg(lock);
            candidates[candidate] += weight;

        },
        0, filters.size(), 1
    );

    if (candidates.empty()){
        logger.log(line_index_str + "No valid OCR candidates. Unable to read number.", COLOR_ORANGE);
        return -1;
    }

    std::pair<int, uint8_t> best;
    for (const auto& item : candidates){
        logger.log(line_index_str + "Candidate " + std::to_string(item.first) + ": " + std::to_string(item.second) + " votes");
        if (item.second > best.second){
            best = item;
        }
    }

    logger.log(line_index_str + "Best candidate: --------------------------> " + std::to_string(best.first));
    return best.first;
}




}
}
