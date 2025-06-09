/*  Console Type Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "NintendoSwitch_ConsoleTypeDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{


ConsoleTypeDetector_Home::ConsoleTypeDetector_Home(ConsoleState& state, Color color)
    : m_state(state)
    , m_color(color)
    , m_bottom_line(0.10, 0.88, 0.80, 0.03)
{}
void ConsoleTypeDetector_Home::make_overlays(VideoOverlaySet& items) const{
    ConsoleType known_state = m_state.console_type();
    if (known_state != ConsoleType::Unknown){
        return;
    }
    items.add(m_color, m_bottom_line);
}
ConsoleType ConsoleTypeDetector_Home::detect(const ImageViewRGB32& screen){
    ConsoleType known_state = m_state.console_type();
    if (known_state != ConsoleType::Unknown){
        return known_state;
    }

    ImageStats stats = image_stats(extract_box_reference(screen, m_bottom_line));
//    cout << "ConsoleTypeDetector: " << stats.stddev.sum() << endl;
    if (stats.stddev.sum() < 10){
        known_state = ConsoleType::Switch2_Unknown;
    }else{
        known_state = ConsoleType::Switch1;
    }

    m_state.set_console_type(known_state);
    return known_state;
}



ConsoleTypeDetector_StartGameUserSelect::ConsoleTypeDetector_StartGameUserSelect(ConsoleState& state, Color color)
    : m_state(state)
    , m_color(color)
    , m_bottom_line(0.02, 0.53, 0.96, 0.03)
{}
void ConsoleTypeDetector_StartGameUserSelect::make_overlays(VideoOverlaySet& items) const{
    ConsoleType known_state = m_state.console_type();
    if (known_state != ConsoleType::Unknown){
        return;
    }
    items.add(m_color, m_bottom_line);
}
ConsoleType ConsoleTypeDetector_StartGameUserSelect::detect(const ImageViewRGB32& screen){
    ConsoleType known_state = m_state.console_type();
    if (known_state != ConsoleType::Unknown){
        return known_state;
    }

    ImageStats stats = image_stats(extract_box_reference(screen, m_bottom_line));
//    cout << "ConsoleTypeDetector: " << stats.stddev.sum() << endl;
    if (stats.stddev.sum() < 10){
        known_state = ConsoleType::Switch2_Unknown;
    }else{
        known_state = ConsoleType::Switch1;
    }

    m_state.set_console_type(known_state);
    return known_state;
}




}
}
