/*  OCR Number Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <vector>
#include <map>
#include "Common/Qt/StringToolsQt.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "CommonFramework/Language.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageFilter.h"
#include "CommonFramework/ImageTools/ImageManip.h"
#include "CommonFramework/ImageTools/BinaryImage_FilterRgb32.h"
#include "OCR_RawOCR.h"
#include "OCR_NumberReader.h"

// #include <iostream>
// using std::cout;
// using std::endl;

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
        {']', '1'},
        {'l', '1'},
        {'i', '1'},
        {'A', '4'},
        {'a', '4'},
        {'S', '5'},
        {'s', '5'},
        {'/', '7'},

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
    for (char32_t ch : to_utf32(input)){
        auto iter = SUBSTITUTION_TABLE.find(ch);
        if (iter == SUBSTITUTION_TABLE.end()){
            continue;
        }
        normalized += iter->second;
    }

    return normalized;
}


int read_number(Logger& logger, const ImageViewRGB32& image, Language language){
    std::string ocr_text = OCR::ocr_read(language, image);
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




int read_number_waterfill(
    Logger& logger, const ImageViewRGB32& image,
    uint32_t rgb32_min, uint32_t rgb32_max
){
    using namespace Kernels::Waterfill;

    //  Direct OCR is unreliable. Instead, we will waterfill each character
    //  to isolate them, then OCR them individually.

    ImageRGB32 filtered = to_blackwhite_rgb32_range(image, rgb32_min, rgb32_max, true);
    PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(image, rgb32_min, rgb32_max);

    std::map<size_t, WaterfillObject> map;
    {
        std::unique_ptr<WaterfillSession> session = make_WaterfillSession(matrix);
        auto iter = session->make_iterator(20);
        WaterfillObject object;
        while (map.size() < 16 && iter->find_next(object, true)){
            map.emplace(object.min_x, std::move(object));
        }
    }

    std::string ocr_text;
    for (const auto& item : map){
        const WaterfillObject& object = item.second;
        ImageRGB32 cropped = extract_box_reference(filtered, object).copy();
        PackedBinaryMatrix tmp(object.packed_matrix());
        filter_by_mask(tmp, cropped, Color(0xffffffff), true);
        ImageRGB32 padded = pad_image(cropped, cropped.width(), 0xffffffff);
        std::string ocr = OCR::ocr_read(Language::English, padded);
        ocr_text += ocr[0];
    }

    std::string normalized = run_number_normalization(ocr_text);

    if (normalized.empty()){
        logger.log("OCR Text: \"" + ocr_text + "\" -> \"" + normalized + "\" -> Unable to read.", COLOR_RED);
        return -1;
    }

    int number = std::atoi(normalized.c_str());
    logger.log("OCR Text: \"" + ocr_text + "\" -> \"" + normalized + "\" -> " + std::to_string(number));

    return number;
}




}
}
