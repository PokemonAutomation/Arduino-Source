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



int read_number(LoggerQt& logger, const QImage& image){
    QString ocr_text = OCR::ocr_read(Language::English, image);
    QString normalized;
    bool has_digit = false;
    for (QChar ch : ocr_text){
        if (ch.isDigit()){
            normalized += ch;
            has_digit = true;
        }
    }

    if (!has_digit){
        return -1;
    }

    int number = normalized.toInt();

    QString str;
    for (QChar ch : ocr_text){
        if (ch != '\r' && ch != '\n'){
            str += ch;
        }
    }
    logger.log("OCR Text: \"" + str + "\" -> \"" + normalized + "\" -> " + QString::number(number));

    return number;
}



}
}
