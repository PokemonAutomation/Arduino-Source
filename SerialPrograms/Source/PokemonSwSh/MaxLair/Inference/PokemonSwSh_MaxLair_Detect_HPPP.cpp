/*  Max Lair Detect PP
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "CommonFramework/OCR/OCR_Filtering.h"
#include "CommonFramework/OCR/OCR_StringNormalization.h"
#include "CommonFramework/OCR/OCR_RawOCR.h"
#include "PokemonSwSh_MaxLair_Detect_HPPP.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{




#if 0
double read_hp_green(const QImage& image){
    int width = image.width();
    int height = image.height();
    int area = width * height;

    ImageStats stats = image_stats(image);
    QRgb color = stats.average.round();
    int bar_R = 0;
    int bar_G = 1;
    int bar_B = 0;

    int bar_area = 0;
    for (int r = 0; r < height; r++){
        for (int c = 0; c < width; c++){
            QRgb pixel = image.pixel(c, r);
            int R = qRed(pixel) - bar_R;
            int G = qGreen(pixel) - bar_G;
            int B = qBlue(pixel) - bar_B;
            if (R*R + G*G + B*B < 100*100){
                bar_area++;
            }
        }
    }

    return (double)bar_area / area;
}
#endif

double read_hp_bar_internal(const QImage& image){
    int width = image.width();
    int height = image.height();
    int area = width * height;

    ImageStats stats;
    double bar = 0.5;
    for (size_t c = 0;; c++){
        stats = image_stats(extract_box(image, ImageFloatBox(0.0, 0.0, bar, 1.0)));
        double max_color = 0;
        max_color = std::max(max_color, stats.average.r);
        max_color = std::max(max_color, stats.average.g);
        max_color = std::max(max_color, stats.average.b);
        if (max_color > 128 && stats.stddev.sum() < 50){
            break;
        }
        bar *= 0.5;
        if (c > 12){
            stats = image_stats(extract_box(image, ImageFloatBox(0.0, 0.0, 1.0, 1.0)));
//            cout << stats.average << stats.stddev << endl;
            image.save("test.png");
            return stats.average.sum() < 384 && stats.stddev.sum() < 80
                ? 0.0
                : -1;
        }
    }

    QRgb color = stats.average.round();
    int bar_R = qRed(color);
    int bar_G = qGreen(color);
    int bar_B = qBlue(color);

    int bar_area = 0;
    for (int r = 0; r < height; r++){
        for (int c = 0; c < width; c++){
            QRgb pixel = image.pixel(c, r);
            int R = qRed(pixel) - bar_R;
            int G = qGreen(pixel) - bar_G;
            int B = qBlue(pixel) - bar_B;
            if (R*R + G*G + B*B < 100*100){
                bar_area++;
            }
        }
    }

    return std::min((double)bar_area / area, bar * 4);
}
double read_hp_bar(const QImage& image){
    //  Try reading just the upper half first.
    double hp = read_hp_bar_internal(extract_box(image, ImageFloatBox(0.5, 0.0, 0.5, 1.0)));
    if (hp > 0){
        return (1.0 + hp) * 0.5;
    }

    //  Now try the bottom half.
    return read_hp_bar_internal(image);
}

double read_hp_bar(Logger& logger, const QImage& image){
    double hp = read_hp_bar(image);
//    static int c = 0;
//    image.save("test-" + QString::number(c++) + ".png");
    if (hp < 0){
        logger.log("HP Read: ?", COLOR_RED);
    }else{
        logger.log("HP Read: " + std::to_string(100 * hp) + "%", COLOR_BLUE);
    }
    return hp;
}
Health read_in_battle_hp_box(Logger& logger, const QImage& sprite, const QImage& hp_bar){
    ImageStats stats = image_stats(sprite);
//    cout << stats.average << stats.stddev << endl;
    if (is_solid(stats, {0., 0.389943, 0.610057})){
        logger.log("HP Read: Dead", COLOR_BLUE);
        return {0, 1};
    }
    double hp = read_hp_bar(logger, hp_bar);
    if (hp == 0){
        return {0, -1};
    }
    return {hp, 0};
}






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
            continue;
        }
        if (ch == 'o' || ch == 'O'){
            tokens.back() += '0';
            continue;
        }
        if (ch == '/'){
            tokens.emplace_back();
            continue;
        }
    }

    if (tokens.size() > 2){
        return -1;
    }
    if (tokens.size() == 2){
        return parse_pp(tokens[0], tokens[1]);
    }

    const std::string& value = tokens[0];
    if (value.empty()){
        return 0;
    }

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
        val = parse_pp(
            value.substr(0, c),
            value.substr(c)
        );
        if (val >= 0){
            return val;
        }
    }

    return -1;
}

int8_t read_pp_text(Logger& logger, QImage image){
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
    logger.log(str, pp < 0 ? COLOR_RED : COLOR_BLUE);

    return pp;
}






























}
}
}
}
