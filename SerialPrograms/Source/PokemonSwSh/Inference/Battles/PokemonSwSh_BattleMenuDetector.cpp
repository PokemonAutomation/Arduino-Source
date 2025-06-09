/*  Battle Menu Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "CommonTools/Images/ColorClustering.h"
#include "PokemonSwSh_BattleMenuDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

//#define DEBUG_StandardBattleMenuDetector

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{



StandardBattleMenuDetector::StandardBattleMenuDetector(
    bool den,
    Color color
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
    , m_status1     (0.165, 0.945, 0.100, 0.015)
{}
void StandardBattleMenuDetector::make_overlays(VideoOverlaySet& items) const{
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
bool StandardBattleMenuDetector::detect(const ImageViewRGB32& screen){
    if (!m_den){
        if (!is_white(extract_box_reference(screen, m_ball_left))){
            return false;
        }
        if (!is_white(extract_box_reference(screen, m_ball_right))){
            return false;
        }
    }else{
        ImageStats health = image_stats(extract_box_reference(screen, m_status1));
        if (!is_white(health)){
//            cout << "Failed: m_status1" << endl;
//            extract_box_reference(screen, m_status1).save("test.png");
            return false;
        }
    }

    bool fight;

#ifdef DEBUG_StandardBattleMenuDetector
    cout << "=============> Fight Text 0" << endl;
#endif
    fight = false;
    fight |= !fight && cluster_fit_2(
        extract_box_reference(screen, m_text_fight),
        Color(0, 0, 0), 0.9,
        Color(255, 255, 255), 0.1
    );
    fight |= !fight && cluster_fit_2(
        extract_box_reference(screen, m_text_fight),
        Color(0, 0, 0), 0.1,
        Color(255, 255, 255), 0.9
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
        extract_box_reference(screen, m_text_pokemon),
        Color(0, 0, 0), 0.1,
        Color(255, 255, 255), 0.9
    );
    fight |= !fight && cluster_fit_2(
        extract_box_reference(screen, m_text_pokemon),
        Color(0, 0, 0), 0.9,
        Color(255, 255, 255), 0.1
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
        extract_box_reference(screen, m_text_bag),
        Color(0, 0, 0), 0.1,
        Color(255, 255, 255), 0.9
    );
    fight |= !fight && cluster_fit_2(
        extract_box_reference(screen, m_text_bag),
        Color(0, 0, 0), 0.9,
        Color(255, 255, 255), 0.1
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
        extract_box_reference(screen, m_text_run),
        Color(0, 0, 0), 0.1,
        Color(255, 255, 255), 0.9
    );
    fight |= !fight && cluster_fit_2(
        extract_box_reference(screen, m_text_run),
        Color(0, 0, 0), 0.9,
        Color(255, 255, 255), 0.1
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
        extract_box_reference(screen, m_icon_fight),
        Color(255, 255, 255), 1.7,
        Color(153, 75, 112), 1.0
    );
#ifdef DEBUG_StandardBattleMenuDetector
    cout << "=============> Fight Symbol 1" << endl;
#endif
    fight |= !fight && cluster_fit_2(
        extract_box_reference(screen, m_icon_fight),
        Color(0, 0, 0), 1.4,
        Color(185, 6, 40), 1.0
    );
    fight |= !fight && cluster_fit_2(   //  Max raid Fight button is a bit different.
        extract_box_reference(screen, m_icon_fight),
        Color(0, 0, 0), 1.7,
        Color(182, 33, 82), 1.0
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
        extract_box_reference(screen, m_icon_pokemon),
        Color(255, 255, 255), 3.1,
        Color(126, 224, 142), 1.0
    );
    pokemon |= !pokemon && cluster_fit_2(
        extract_box_reference(screen, m_icon_pokemon),
        Color(0, 0, 0), 2.7,
        Color(8, 158, 18), 1.0
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
        extract_box_reference(screen, m_icon_bag),
        Color(255, 255, 255), 2.4,
        Color(236, 192, 124), 1.0
    );
    bag |= !bag && cluster_fit_2(
        extract_box_reference(screen, m_icon_bag),
        Color(0, 0, 0), 1.9,
        Color(215, 120, 11), 1.0
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
        extract_box_reference(screen, m_icon_run),
        Color(255, 255, 255), 2.3,
        Color(216, 150, 230), 1.0
    );
    run |= !run && cluster_fit_2(
        extract_box_reference(screen, m_icon_run),
        Color(0, 0, 0), 1.9,
        Color(179, 15, 195), 1.0
    );
    if (!run){
//        cout << "Failed: m_icon_run" << endl;
        return false;
    }
#endif

//    image.save("battle-menu.png");
    return true;
}


StandardBattleMenuWatcher::StandardBattleMenuWatcher(bool den, Color color)
    : StandardBattleMenuDetector(den, color)
    , VisualInferenceCallback("StandardBattleMenuWatcher")
{}
void StandardBattleMenuWatcher::make_overlays(VideoOverlaySet& items) const{
    return StandardBattleMenuDetector::make_overlays(items);
}
bool StandardBattleMenuWatcher::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
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
