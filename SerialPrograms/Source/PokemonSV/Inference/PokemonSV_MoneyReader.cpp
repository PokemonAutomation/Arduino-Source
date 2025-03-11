/*  Money Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <regex>
#include "Common/Cpp/AbstractLogger.h"
#include "CommonFramework/Language.h"
#include "CommonTools/OCR/OCR_RawOCR.h"
#include "PokemonSV_MoneyReader.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace OCR{



int read_money(Logger& logger, const ImageViewRGB32& image){
    std::string ocr_text = OCR::ocr_read(Language::English, image);
    std::string normalized;

    //logger.log("OCR Text: " + ocr_text);

    //Clean up the string because regex search does not like accented characters
    std::regex regg("[^a-zA-Z0-9\xA3, .#]+");
    ocr_text = std::regex_replace(ocr_text, regg, "");

    //Find pound sign and amount, we want to prevent cases like the following:
    //OCR Text: "Al IRt 1N} . !You got 27,200 in prize money!" -> "4127200" -> 4127200
    //OCR Text: "You got 26,400 in prize money!AR -" -> "264004" -> 264004
    //OCR Text: "v N r 1 .You got 26,400 in prize money!" -> "126400" -> 126400
    std::regex reg{ "\xA3[a-zA-Z0-9,.]+| [0-9,. ]+(\xA3|#)" };
    std::smatch match;
    std::regex_search(ocr_text, match, reg);
    std::ssub_match sub_match = match[0];

    bool has_digit = false;
    for (char ch : sub_match.str()){
        //  4 is commonly misread as A.
        if (ch == 'a' || ch == 'A'){
            normalized += '4';
            has_digit = true;
        }
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

    logger.log("OCR Text: \"" + str + "\" -> \"" + sub_match.str() + "\" -> " + normalized + "\" -> " + std::to_string(number));

    return number;
}



}
}
