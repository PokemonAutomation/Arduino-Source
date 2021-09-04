/*  Max Lair Detect Battle Menu
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Compiler.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "CommonFramework/ImageTools/ColorClustering.h"
#include "PokemonSwSh_MaxLair_Detect_BattleMenu.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


BattleMenuDetector::BattleMenuDetector(VideoOverlay& overlay)
    : m_icon_fight  (overlay, 0.923, 0.576 + 1 * 0.1075, 0.05, 0.080, Qt::yellow)
    , m_icon_pokemon(overlay, 0.923, 0.576 + 2 * 0.1075, 0.05, 0.080, Qt::yellow)
    , m_icon_run    (overlay, 0.923, 0.576 + 3 * 0.1075, 0.05, 0.080, Qt::yellow)
    , m_text_fight  (overlay, 0.830, 0.576 + 1 * 0.1075, 0.08, 0.080, Qt::yellow)
    , m_text_pokemon(overlay, 0.830, 0.576 + 2 * 0.1075, 0.08, 0.080, Qt::yellow)
    , m_text_run    (overlay, 0.830, 0.576 + 3 * 0.1075, 0.08, 0.080, Qt::yellow)
//    , m_info_left   (overlay, 0.907, 0.500, 0.02, 0.03, Qt::yellow)
//    , m_info_right  (overlay, 0.970, 0.500, 0.02, 0.03, Qt::yellow)
    , m_status      (overlay, 0.165, 0.945, 0.100, 0.020, Qt::yellow)
{}
bool BattleMenuDetector::on_frame(
    const QImage& frame,
    std::chrono::system_clock::time_point timestamp
){
    return detect(frame);
}


bool BattleMenuDetector::detect(const QImage& screen){
    bool fight;

    fight = false;
    fight |= !fight && cluster_fit_2(
        extract_box(screen, m_text_fight),
        qRgb(0, 0, 0), 0.9,
        qRgb(255, 255, 255), 0.1,
        0.2, 50, 0.1
    );
    fight |= !fight && cluster_fit_2(
        extract_box(screen, m_text_fight),
        qRgb(0, 0, 0), 0.1,
        qRgb(255, 255, 255), 0.9,
        0.2, 50, 0.1
    );
    if (!fight){
        return false;
    }

    fight = false;
    fight |= !fight && cluster_fit_2(
        extract_box(screen, m_text_pokemon),
        qRgb(0, 0, 0), 0.1,
        qRgb(255, 255, 255), 0.9,
        0.2, 50, 0.1
    );
    fight |= !fight && cluster_fit_2(
        extract_box(screen, m_text_pokemon),
        qRgb(0, 0, 0), 0.9,
        qRgb(255, 255, 255), 0.1,
        0.2, 50, 0.1
    );
    if (!fight){
        return false;
    }

    fight = false;
    fight |= !fight && cluster_fit_2(
        extract_box(screen, m_text_run),
        qRgb(0, 0, 0), 0.1,
        qRgb(255, 255, 255), 0.9,
        0.2, 50, 0.1
    );
    fight |= !fight && cluster_fit_2(
        extract_box(screen, m_text_run),
        qRgb(0, 0, 0), 0.9,
        qRgb(255, 255, 255), 0.1,
        0.2, 50, 0.1
    );
    if (!fight){
        return false;
    }


    fight = false;
    fight |= !fight && cluster_fit_2(
        extract_box(screen, m_icon_fight),
        qRgb(255, 255, 255), 1.7,
        qRgb(153, 75, 112), 1.0
    );
    fight |= !fight && cluster_fit_2(
        extract_box(screen, m_icon_fight),
        qRgb(0, 0, 0), 1.4,
        qRgb(185, 6, 40), 1.0
    );
    fight |= !fight && cluster_fit_2(   //  Max raid Fight button is a bit different.
        extract_box(screen, m_icon_fight),
        qRgb(0, 0, 0), 1.7,
        qRgb(182, 33, 82), 1.0
    );
//    cout << "===============" << endl;
    if (!fight){
        fight = cluster_fit_2(   //  Cheer
            extract_box(screen, m_icon_fight),
            qRgb(0, 0, 0), 2.2,
            qRgb(9, 162, 218), 1.0
        );
        m_cheer = fight;
    }
    if (!fight){
        return false;
    }

    bool pokemon = false;
    pokemon |= !pokemon && cluster_fit_2(
        extract_box(screen, m_icon_pokemon),
        qRgb(255, 255, 255), 3.1,
        qRgb(126, 224, 142), 1.0
    );
    pokemon |= !pokemon && cluster_fit_2(
        extract_box(screen, m_icon_pokemon),
        qRgb(0, 0, 0), 2.7,
        qRgb(8, 158, 18), 1.0
    );
    if (!pokemon){
        return false;
    }

    bool run = false;
    run |= !run && cluster_fit_2(
        extract_box(screen, m_icon_run),
        qRgb(255, 255, 255), 2.3,
        qRgb(216, 150, 230), 1.0
    );
    run |= !run && cluster_fit_2(
        extract_box(screen, m_icon_run),
        qRgb(0, 0, 0), 1.9,
        qRgb(179, 15, 195), 1.0
    );
    if (!run){
        return false;
    }


    //  Check for white status bar in bottom left corner.
    ImageStats health = image_stats(extract_box(screen, m_status));
//    cout << health.average << ", " << health.stddev << endl;
    if (is_grey(health, 400, 1000)){
        return true;
    }

    //  Check the semi-transparent red status bar if you're dmaxed.


//    image.save("battle-menu.png");
    return false;
}



double read_hp_bar(const QImage& image){
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
        if (max_color > 128 && stats.stddev.sum() < 20){
            break;
        }
        bar *= 0.5;
        if (c > 12){
            return -1;
        }
    }

#if 0
    int bar_R = 255;
    int bar_G = 0;
    int bar_B = 0;
    int divider = 4;

    ImageStats stats = image_stats(extract_box(image, ImageFloatBox(0.0, 0.0, 0.5, 1.0)));
//    cout << stats.average << " - " << stats.stddev << endl;
    if (is_solid(stats, {0, 1, 0}, 0.1, 20) && stats.average.sum() >= 128){
//        cout << "green" << endl;
        QRgb color = stats.average.round();
        bar_R = qRed(color);
        bar_G = qGreen(color);
        bar_B = qBlue(color);
        divider = 1;
    }

    stats = image_stats(extract_box(image, ImageFloatBox(0.0, 0.0, 0.25, 1.0)));
//    cout << stats.average << " - " << stats.stddev << endl;
    if (is_solid(stats, {0.564641, 0.435359, 0.}, 0.1, 20) && stats.average.sum() >= 256){
//        cout << "yellow" << endl;
        QRgb color = stats.average.round();
        bar_R = qRed(color);
        bar_G = qGreen(color);
        bar_B = qBlue(color);
        divider = 2;
    }

    width /= divider;
#endif

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



BattleMenuReader::BattleMenuReader(VideoOverlay& overlay)
    : m_opponent_hp(overlay, 0.360, 0.120, 0.280, 0.005)
{}

double BattleMenuReader::read_opponent_hp(Logger& logger, const QImage& screen) const{
    QImage image = extract_box(screen, m_opponent_hp);
//    image.save("test.png");

//    ImageStats stats = image_stats(image);
//    cout << stats.average << " - " << stats.stddev << endl;

    double hp = read_hp_bar(image);
    logger.log("Reading HP: " + (hp < 0 ? "?" : std::to_string(100 * hp)) + "%");
    if (hp <= 0){
        dump_image(logger, screen, "BattleMenuReader-read_opponent_hp");
    }
    return hp;
}































}
}
}
}
