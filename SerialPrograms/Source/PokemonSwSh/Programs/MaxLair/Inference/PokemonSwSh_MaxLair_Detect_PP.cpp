/*  Max Lair Detect PP
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/OCR/Filtering.h"
#include "CommonFramework/OCR/StringNormalization.h"
#include "CommonFramework/OCR/RawOCR.h"
#include "PokemonSwSh_MaxLair_Detect_PP.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


int8_t parse_pp(const std::string& current, const std::string& total){
//    cout << current << " " << total << endl;
    if (total[0] == '0'){
        return -1;
    }
    uint32_t num_current = atoi(current.c_str());
    uint32_t num_total = atoi(total.c_str());
    if (num_current > num_total){
        return -1;
    }
    if (num_current > 64 || num_total > 64){
        return -1;
    }
    if (num_total == 1){
        return (int8_t)num_current;
    }
    if (num_total % 5 != 0){
        return -1;
    }
    return (int8_t)num_current;
}
int8_t parse_pp(const std::string& str){
    //  Clean up and split into tokens deliminated by '/'.
    std::vector<std::string> tokens;
    tokens.emplace_back();
    for (char ch : str){
        if (ch <= 32){
            continue;
        }
        if ('0' <= ch && ch <= '9'){
            tokens.back() += ch;
        }
        if (ch == 'o' || ch == 'O'){
            tokens.back() += '0';
        }
        if (ch == '/'){
            tokens.emplace_back();
        }
    }

    if (tokens.size() > 2){
        return -1;
    }
    if (tokens.size() == 2){
        return parse_pp(tokens[0], tokens[1]);
    }

    const std::string& value = tokens[0];
    for (size_t c = value.size(); c > 1;){
        char ch = value[--c];
        if (ch != '1' && ch != '7'){
            continue;
        }
        int8_t val = parse_pp(
            value.substr(0, c),
            value.substr(c + 1)
        );
        if (val >= 0){
            return val;
        }
    }

    return -1;
}

int8_t read_pp_from_swap_menu(Logger& logger, QImage image){
    OCR::binary_filter_solid_background(image);

    QString ocr_text = OCR::ocr_read(Language::English, image);

    ocr_text = OCR::run_character_reductions(ocr_text);
    int8_t pp = parse_pp(ocr_text.toStdString());

    QString str;
    str += "OCR Result: \"";
    for (QChar ch : ocr_text){
        if (ch != '\r' && ch != '\n'){
            str += ch;
        }
    }
    str += "\" -> (";
    str += pp < 0 ? "? PP)" : QString::number((int)pp) + " PP)";
    logger.log(str, pp < 0 ? Qt::red : Qt::blue);

    return pp;
}


}
}
}
}
