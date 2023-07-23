/*  OCR Number Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <map>
#include "CommonFramework/Language.h"
#include "OCR_RawOCR.h"
#include "OCR_NumberReader.h"

// #include <iostream>
// using std::cout;
// using std::endl;

namespace PokemonAutomation{
namespace OCR{



int read_number(Logger& logger, const ImageViewRGB32& image){
    static const std::map<char, char> SUBSTITUTION_TABLE{
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
        {'l', '1'},
        {'i', '1'},
        {'A', '4'},
        {'a', '4'},
        {'S', '5'},
        {'s', '5'},
    };

    std::string ocr_text = OCR::ocr_read(Language::English, image);
    std::string normalized;
    bool has_digit = false;
    for (char ch : ocr_text){
        auto iter = SUBSTITUTION_TABLE.find(ch);
        if (iter == SUBSTITUTION_TABLE.end()){
            continue;
        }
        normalized += iter->second;
        has_digit = true;
    }

    std::string str;
    for (char ch : ocr_text){
        if (ch != '\r' && ch != '\n'){
            str += ch;
        }
    }

    if (!has_digit){
        logger.log("OCR Text: \"" + str + "\" -> \"" + normalized + "\" -> Unable to read.", COLOR_RED);
        return -1;
    }

    int number = std::atoi(normalized.c_str());
    logger.log("OCR Text: \"" + str + "\" -> \"" + normalized + "\" -> " + std::to_string(number));

    return number;
}



}
}
