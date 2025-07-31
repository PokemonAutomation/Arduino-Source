/*  Date Change Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "NintendoSwitch_DateChangeDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{




DateChangeDetector_Switch1::DateChangeDetector_Switch1(Color color)
    : m_color(color)
    , m_background_top(0.50, 0.02, 0.45, 0.08)
    , m_window_top(0.50, 0.36, 0.45, 0.07)
    , m_window_text(0.05, 0.36, 0.10, 0.07)
    , m_jp_year(0.136, 0.61, 0.11, 0.09)
    , m_us_hour(0.473, 0.61, 0.06, 0.09)
    , m_jp_month_arrow(0.30, 0.50, 0.05, 0.06)
{}
void DateChangeDetector_Switch1::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_background_top);
    items.add(m_color, m_window_top);
    items.add(m_color, m_window_text);
    items.add(m_color, m_jp_year);
    items.add(m_color, m_us_hour);
    items.add(m_color, m_jp_month_arrow);
}
bool DateChangeDetector_Switch1::detect(const ImageViewRGB32& screen){
    ImageStats stats_background_top = image_stats(extract_box_reference(screen, m_background_top));
    if (stats_background_top.stddev.sum() > 10){
//        cout << "asdf" << endl;
        return false;
    }
    ImageStats stats_window_top = image_stats(extract_box_reference(screen, m_window_top));
//    cout << stats_window_top.average << stats_window_top.stddev << endl;
    if (stats_window_top.stddev.sum() > 10){
//        cout << "qwer" << endl;
        return false;
    }
    ImageStats stats_window_text = image_stats(extract_box_reference(screen, m_window_text));
//    cout << stats_window_text.stddev << endl;
    if (stats_window_text.stddev.sum() < 100){
//        cout << "zxcv" << endl;
        return false;
    }
//    cout << "stats_background_top: " << stats_background_top.average.sum() << endl;
//    cout << "stats_window_top: " << stats_window_top.average.sum() << endl;
    if (std::abs(stats_background_top.average.sum() - stats_window_top.average.sum()) < 50){
//        cout << "xcvb" << endl;
        return false;
    }

//    bool white_theme = stats_window_top.average.sum() > 600;

    ImageViewRGB32 year_box = extract_box_reference(screen, m_jp_year);
    ImageStats year_stats = image_stats(year_box);
//    cout << year_stats.average << year_stats.stddev << endl;

    double stddev = year_stats.stddev.sum();
    if (stddev < 80){
//        cout << "sdfg" << endl;
        return false;
    }

    return true;
}
DateFormat DateChangeDetector_Switch1::detect_date_format(const ImageViewRGB32& screen) const{
    ImageViewRGB32 us_hours = extract_box_reference(screen, m_us_hour);
    ImageStats stats_us_hours = image_stats(us_hours);

    if (stats_us_hours.stddev.sum() > 30){
        return DateFormat::US;
    }

    ImageViewRGB32 jp_arrow = extract_box_reference(screen, m_jp_month_arrow);
    ImageStats stats_arrow = image_stats(jp_arrow);
    if (stats_arrow.stddev.sum() > 30){
        return DateFormat::JP;
    }

    return DateFormat::EU;
}



DateChangeDetector_Switch2::DateChangeDetector_Switch2(Color color)
    : m_color(color)
    , m_background_top(0.50, 0.02, 0.45, 0.08)
    , m_window_bottom(0.50, 0.80, 0.45, 0.07)
    , m_window_text(0.05, 0.02, 0.10, 0.08)
    , m_jp_year(0.139, 0.436, 0.088, 0.095)
    , m_us_hour(0.466856, 0.567340, 0.049242, 0.063973)
    , m_jp_month_arrow(0.291705, 0.331675, 0.054986, 0.069652)
{}
void DateChangeDetector_Switch2::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_background_top);
    items.add(m_color, m_window_bottom);
    items.add(m_color, m_window_text);
    items.add(m_color, m_jp_year);
    items.add(m_color, m_us_hour);
    items.add(m_color, m_jp_month_arrow);
}
bool DateChangeDetector_Switch2::detect(const ImageViewRGB32& screen){
    ImageStats stats_background_top = image_stats(extract_box_reference(screen, m_background_top));
    if (stats_background_top.stddev.sum() > 10){
//        cout << "asdf" << endl;
        return false;
    }
    ImageStats stats_window_bottom = image_stats(extract_box_reference(screen, m_window_bottom));
//    cout << stats_window_bottom.average << stats_window_top.stddev << endl;
    if (stats_window_bottom.stddev.sum() > 10){
//        cout << "qwer" << endl;
        return false;
    }
    ImageStats stats_window_text = image_stats(extract_box_reference(screen, m_window_text));
//    cout << stats_window_text.stddev << endl;
    if (stats_window_text.stddev.sum() < 100){
//        cout << "zxcv" << endl;
        return false;
    }
//    cout << "stats_background_top: " << stats_background_top.average.sum() << endl;
//    cout << "stats_window_top: " << stats_window_top.average.sum() << endl;

    if (euclidean_distance(stats_background_top.average, stats_window_bottom.average) > 10){
//        cout << "xcvb" << endl;
        return false;
    }

//    bool white_theme = stats_window_top.average.sum() > 600;

    ImageViewRGB32 year_box = extract_box_reference(screen, m_jp_year);
    ImageStats year_stats = image_stats(year_box);
//    cout << year_stats.average << year_stats.stddev << endl;

    double stddev = year_stats.stddev.sum();
    if (stddev < 80){
//        cout << "sdfg" << endl;
        return false;
    }

    return true;
}
DateFormat DateChangeDetector_Switch2::detect_date_format(const ImageViewRGB32& screen) const{
    ImageViewRGB32 us_hours = extract_box_reference(screen, m_us_hour);
    ImageStats stats_us_hours = image_stats(us_hours);

//    cout << stats_us_hours.stddev.sum() << endl;

    if (stats_us_hours.stddev.sum() > 30){
        return DateFormat::US;
    }

    ImageViewRGB32 jp_arrow = extract_box_reference(screen, m_jp_month_arrow);
    ImageStats stats_arrow = image_stats(jp_arrow);
    if (stats_arrow.stddev.sum() > 30){
        return DateFormat::JP;
    }

    return DateFormat::EU;
}







}
}
