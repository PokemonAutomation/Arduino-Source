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


ConsoleTypeDetector_Home::ConsoleTypeDetector_Home(ConsoleHandle& console, Color color)
    : m_console(console)
    , m_color(color)
    , m_bottom_line(0.10, 0.88, 0.80, 0.03)
{}
void ConsoleTypeDetector_Home::make_overlays(VideoOverlaySet& items) const{
    ConsoleType known_state = m_console.state().console_type();
    if (known_state != ConsoleType::Unknown){
        return;
    }
    items.add(m_color, m_bottom_line);
}
ConsoleType ConsoleTypeDetector_Home::detect(const ImageViewRGB32& screen){
    if (m_console.state().console_type_confirmed()){
        return m_console.state().console_type();
    }

    ImageStats stats = image_stats(extract_box_reference(screen, m_bottom_line));
//    cout << "ConsoleTypeDetector: " << stats.stddev.sum() << endl;
    ConsoleType state;
    if (stats.stddev.sum() < 10){
        state = ConsoleType::Switch2_Unknown;
    }else{
        state = ConsoleType::Switch1;
    }

    m_console.state().set_console_type(m_console, state);
    return state;
}



ConsoleTypeDetector_StartGameUserSelect::ConsoleTypeDetector_StartGameUserSelect(ConsoleHandle& console, Color color)
    : m_console(console)
    , m_color(color)
    , m_bottom_line(0.02, 0.53, 0.96, 0.03)
{}
void ConsoleTypeDetector_StartGameUserSelect::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_bottom_line);
}
ConsoleType ConsoleTypeDetector_StartGameUserSelect::detect(const ImageViewRGB32& screen){
    if (m_console.state().console_type_confirmed()){
        return m_console.state().console_type();
    }

    ImageStats stats = image_stats(extract_box_reference(screen, m_bottom_line));
//    cout << "ConsoleTypeDetector: " << stats.stddev.sum() << endl;
    ConsoleType state;
    if (stats.stddev.sum() < 10){
        state = ConsoleType::Switch2_Unknown;
    }else{
        state = ConsoleType::Switch1;
    }

    m_console.state().set_console_type(m_console, state);
    return state;
}




}
}
