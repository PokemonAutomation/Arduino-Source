/*  OCR Number Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Language.h"
#include "OCR_RawOCR.h"
#include "OCR_NumberReader.h"

namespace PokemonAutomation{
namespace OCR{



int read_number(LoggerQt& logger, const ImageViewRGB32& image){
    std::string ocr_text = OCR::ocr_read(Language::English, image);
    std::string normalized;
    bool has_digit = false;
    for (char ch : ocr_text){
        if ('0' <= ch && ch <= '9'){
            normalized += ch;
            has_digit = true;
        }
    }

    if (!has_digit){
        return -1;
    }

    int number = std::atoi(normalized.c_str());

    std::string str;
    for (char ch : ocr_text){
        if (ch != '\r' && ch != '\n'){
            str += ch;
        }
    }
    logger.log("OCR Text: \"" + str + "\" -> \"" + normalized + "\" -> " + std::to_string(number));

    return number;
}



}
}
