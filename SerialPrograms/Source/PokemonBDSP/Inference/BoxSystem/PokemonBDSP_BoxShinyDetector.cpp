/*  Box Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QImage>
#include "CommonFramework/Tools/VideoOverlaySet.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "PokemonBDSP_BoxShinyDetector.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


BoxShinyDetector::BoxShinyDetector(Color color)
    : m_color(color)
    , m_symbol(0.960, 0.145, 0.030, 0.050)
    , m_background(0.960, 0.200, 0.030, 0.400)
{}
void BoxShinyDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_symbol);
    items.add(m_color, m_background);
}

bool BoxShinyDetector::is_panel(const QImage& screen) const{
    ImageStats background = image_stats(extract_box(screen, m_background));
//    cout << background.average << background.stddev << endl;
    if (!is_white(background)){
        return false;
    }
    return true;
}
bool BoxShinyDetector::detect(const QImage& screen) const{
    if (!is_panel(screen)){
        return false;
    }
    ImageStats symbol = image_stats(extract_box(screen, m_symbol));
//    extract_box(screen, m_symbol).save("test.png");
//    cout << symbol.average << symbol.stddev << endl;

    double average = symbol.average.sum();
    double stddev = symbol.stddev.sum();
    if (stddev < 50){
//        cout << "bad stddev = " << stddev << endl;
        return false;
    }

    FloatPixel actual = symbol.average / average;
    double distance = euclidean_distance(actual, {0.366743, 0.320479, 0.312778});
//    cout << "distance = " << distance << endl;
    return distance <= 0.15;
}






}
}
}
