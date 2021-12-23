/*  Battle Menu Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Compiler.h"
#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "CommonFramework/ImageTools/ColorClustering.h"
#include "CommonFramework/Inference/ImageTools.h"
#include "PokemonSwSh_BattleMenuDetector.h"

#include <iostream>
using std::cout;
using std::endl;

//#define DEBUG_StandardBattleMenuDetector

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{



StandardBattleMenuDetector::StandardBattleMenuDetector(
    bool den,
    QColor color
)
    : m_den(den)
    , m_color(color)
    , m_ball_left   (0.912, 0.452, 0.02, 0.03)
    , m_ball_right  (0.970, 0.452, 0.02, 0.03)
    , m_icon_fight  (0.923, 0.576 + 0 * 0.1075, 0.05, 0.080)
    , m_icon_pokemon(0.923, 0.576 + 1 * 0.1075, 0.05, 0.080)
    , m_icon_bag    (0.923, 0.576 + 2 * 0.1075, 0.05, 0.080)
    , m_icon_run    (0.923, 0.576 + 3 * 0.1075, 0.05, 0.080)
    , m_text_fight  (0.830, 0.576 + 0 * 0.1075, 0.08, 0.080)
    , m_text_pokemon(0.830, 0.576 + 1 * 0.1075, 0.08, 0.080)
    , m_text_bag    (0.830, 0.576 + 2 * 0.1075, 0.08, 0.080)
    , m_text_run    (0.830, 0.576 + 3 * 0.1075, 0.08, 0.080)
//    , m_status0     (0.280, 0.870, 0.015, 0.030)
    , m_status1     (0.165, 0.945, 0.100, 0.020)
{}
void StandardBattleMenuDetector::make_overlays(OverlaySet& items) const{
    if (!m_den){
        items.add(m_color, m_ball_left);
        items.add(m_color, m_ball_right);
    }else{
        items.add(m_color, m_status1);
    }
    items.add(m_color, m_icon_fight);
    items.add(m_color, m_icon_pokemon);
    items.add(m_color, m_icon_bag);
    items.add(m_color, m_icon_run);
    items.add(m_color, m_text_fight);
    items.add(m_color, m_text_pokemon);
    items.add(m_color, m_text_bag);
    items.add(m_color, m_text_run);
}
bool StandardBattleMenuDetector::detect(const QImage& screen) const{
    if (!m_den){
        if (!is_white(extract_box(screen, m_ball_left))){
            return false;
        }
        if (!is_white(extract_box(screen, m_ball_right))){
            return false;
        }
    }else{
        ImageStats health = image_stats(extract_box(screen, m_status1));
        if (!is_white(health)){
//            cout << "Failed: m_status1" << endl;
            return false;
        }
    }

    bool fight;

#ifdef DEBUG_StandardBattleMenuDetector
    cout << "=============> Fight Text 0" << endl;
#endif
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
//        cout << "Failed: m_text_fight" << endl;
        return false;
    }

#ifdef DEBUG_StandardBattleMenuDetector
    cout << "=============> Pokemon Text" << endl;
#endif
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
//        cout << "Failed: m_text_pokemon" << endl;
        return false;
    }

#ifdef DEBUG_StandardBattleMenuDetector
    cout << "=============> Bag Text" << endl;
#endif
    fight = false;
    fight |= !fight && cluster_fit_2(
        extract_box(screen, m_text_bag),
        qRgb(0, 0, 0), 0.1,
        qRgb(255, 255, 255), 0.9,
        0.2, 50, 0.1
    );
    fight |= !fight && cluster_fit_2(
        extract_box(screen, m_text_bag),
        qRgb(0, 0, 0), 0.9,
        qRgb(255, 255, 255), 0.1,
        0.2, 50, 0.1
    );
    if (!fight){
//        cout << "Failed: m_text_bag" << endl;
        return false;
    }

#ifdef DEBUG_StandardBattleMenuDetector
    cout << "=============> Run Text" << endl;
#endif
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
//        cout << "Failed: m_text_run" << endl;
        return false;
    }


#if 1
#ifdef DEBUG_StandardBattleMenuDetector
    cout << "=============> Fight Symbol 0" << endl;
#endif
    fight = false;
    fight |= !fight && cluster_fit_2(
        extract_box(screen, m_icon_fight),
        qRgb(255, 255, 255), 1.7,
        qRgb(153, 75, 112), 1.0
    );
#ifdef DEBUG_StandardBattleMenuDetector
    cout << "=============> Fight Symbol 1" << endl;
#endif
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
    if (!fight){
//        cout << "Failed: m_icon_fight" << endl;
        return false;
    }

#ifdef DEBUG_StandardBattleMenuDetector
    cout << "=============> Pokemon Symbol" << endl;
#endif
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
//        cout << "Failed: m_icon_pokemon" << endl;
        return false;
    }

#ifdef DEBUG_StandardBattleMenuDetector
    cout << "=============> Bag Symbol" << endl;
#endif
    bool bag = false;
    bag |= !bag && cluster_fit_2(
        extract_box(screen, m_icon_bag),
        qRgb(255, 255, 255), 2.4,
        qRgb(236, 192, 124), 1.0
    );
    bag |= !bag && cluster_fit_2(
        extract_box(screen, m_icon_bag),
        qRgb(0, 0, 0), 1.9,
        qRgb(215, 120, 11), 1.0
    );
    if (!bag){
//        cout << "Failed: m_icon_bag" << endl;
        return false;
    }

#ifdef DEBUG_StandardBattleMenuDetector
    cout << "=============> Run Symbol" << endl;
#endif
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
//        cout << "Failed: m_icon_run" << endl;
        return false;
    }
#endif

//    image.save("battle-menu.png");
    return true;
}


void StandardBattleMenuWatcher::make_overlays(OverlaySet& items) const{
    return StandardBattleMenuDetector::make_overlays(items);
}
bool StandardBattleMenuWatcher::process_frame(
    const QImage& frame,
    std::chrono::system_clock::time_point timestamp
){
    //  Need 5 consecutive successful detections.
    if (!detect(frame)){
        m_trigger_count = 0;
        return false;
    }
    m_trigger_count++;
    return m_trigger_count >= 5;
}





}
}
}
