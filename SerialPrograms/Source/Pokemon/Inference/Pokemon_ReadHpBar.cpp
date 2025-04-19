/*  Read HP Bar
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/AbstractLogger.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "Pokemon_ReadHpBar.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace Pokemon{


double read_hp_bar_internal(const ImageViewRGB32& image){
    size_t width = image.width();
    size_t height = image.height();
    size_t area = width * height;

//    static int i = 0;
//    image.save("test-" + std::to_string(++i) + ".png");
//    cout << "start: " << i << endl;

    ImageStats stats;
    double bar = 0.5;
    for (size_t c = 0;; c++){
        stats = image_stats(extract_box_reference(image, ImageFloatBox(0.0, 0.0, bar, 1.0)));
        double max_color = 0;
        max_color = std::max(max_color, stats.average.r);
        max_color = std::max(max_color, stats.average.g);
        max_color = std::max(max_color, stats.average.b);
//        cout << "max_color: " << max_color << ", stddev: " << stats.stddev.sum() << endl;
        if (max_color > 128 && stats.stddev.sum() < 120){
            break;
        }
        bar *= 0.5;
        if (c > 12){
            stats = image_stats(extract_box_reference(image, ImageFloatBox(0.0, 0.0, 1.0, 1.0)));
//            cout << stats.average << stats.stddev << endl;
//            image.save("test.png");
            return stats.average.sum() < 384 && stats.stddev.sum() < 80
                ? 0.0
                : -1;
        }
    }
//    cout << "end: " << i << endl;

    Color color = stats.average.round();
    int bar_R = color.red();
    int bar_G = color.green();
    int bar_B = color.blue();

    int bar_area = 0;
    for (size_t r = 0; r < height; r++){
        for (size_t c = 0; c < width; c++){
            Color pixel(image.pixel(c, r));
            int R = pixel.red() - bar_R;
            int G = pixel.green() - bar_G;
            int B = pixel.blue() - bar_B;
            if (R*R + G*G + B*B < 100*100){
                bar_area++;
            }
        }
    }

    return std::min((double)bar_area / area, bar * 4);
}
double read_hp_bar(const ImageViewRGB32& image){
    //  Try reading just the upper half first.
    double hp = read_hp_bar_internal(extract_box_reference(image, ImageFloatBox(0.5, 0.0, 0.5, 1.0)));
    if (hp > 0){
        return (1.0 + hp) * 0.5;
    }

    //  Now try the bottom half.
    return read_hp_bar_internal(image);
}

double read_hp_bar(Logger& logger, const ImageViewRGB32& image){
    double hp = read_hp_bar(image);

//    static int c = 0;
//    image.save("test-" + std::to_string(c++) + ".png");

    if (hp <= 0){
        logger.log("HP Read: ?", COLOR_RED);
    }else{
        logger.log("HP Read: " + std::to_string(100 * hp) + "%", COLOR_BLUE);
    }
    return hp;
}



}
}
