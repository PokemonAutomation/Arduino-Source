/*  Max Lair Detect Lobby
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Compiler.h"
#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "PokemonSwSh_MaxLair_Detect_Lobby.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


#if 0
LobbyReadyButtonDetector::LobbyReadyButtonDetector(VideoOverlay& overlay)
    : m_box0(overlay, 0.630, 0.695, 0.034, 0.04)
    , m_box1(overlay, 0.910, 0.695, 0.034, 0.04)
{}
bool LobbyReadyButtonDetector::on_frame(
    const QImage& frame,
    std::chrono::system_clock::time_point timestamp
){
    return detect(frame);
}
bool LobbyReadyButtonDetector::detect(const QImage& screen){
    ImageStats stats0 = pixel_stats(extract_box(screen, m_box0));
    ImageStats stats1 = pixel_stats(extract_box(screen, m_box1));
    return is_white(stats0) && is_white(stats1);
}
#endif


#if 0
NonLobbyDetector::NonLobbyDetector(VideoOverlay& overlay)
    : m_pink (overlay, 0.575, 0.035, 0.050, 0.100)
    , m_white(overlay, 0.800, 0.200, 0.150, 0.100)
{}
bool NonLobbyDetector::on_frame(
    const QImage& frame,
    std::chrono::system_clock::time_point timestamp
){
    return detect(frame);
}
bool NonLobbyDetector::detect(const QImage& screen){
    ImageStats stats0 = pixel_stats(extract_box(screen, m_pink));
    ImageStats stats1 = pixel_stats(extract_box(screen, m_white));
//    cout << stats0.average << ", " << stats0.stddev << endl;
//    cout << stats1.average << ", " << stats1.stddev << endl;
    if (!is_solid(stats0, {0.444944, 0.150562, 0.404494})){
        return true;
    }
    if (!is_solid(stats1, {0.303079, 0.356564, 0.340357})){
        return true;
    }
    return false;
}
#endif


LobbyDoneConnecting::LobbyDoneConnecting()
    : m_box(0.600, 0.820, 0.080, 0.100)
{
    add_box(m_box);
}
bool LobbyDoneConnecting::process_frame(
    const QImage& frame,
    std::chrono::system_clock::time_point timestamp
){
    return detect(frame);
}
bool LobbyDoneConnecting::detect(const QImage& screen){
    ImageStats stats0 = image_stats(extract_box(screen, m_box));
//    cout << stats0.average << ", " << stats0.stddev << endl;
    if (!is_grey(stats0, 0, 200)){
        return true;
    }
    return false;
}


LobbyAllReadyDetector::LobbyAllReadyDetector(size_t consoles)
    : m_consoles(consoles)
    , m_checkbox0(0.669, 0.337 + 0.0775*0, 0.034, 0.06)
    , m_checkbox1(0.669, 0.337 + 0.0775*1, 0.034, 0.06)
    , m_checkbox2(0.669, 0.337 + 0.0775*2, 0.034, 0.06)
    , m_checkbox3(0.669, 0.337 + 0.0775*3, 0.034, 0.06)
{
    add_box(m_checkbox0);
    add_box(m_checkbox1);
    add_box(m_checkbox2);
    add_box(m_checkbox3);
}
bool LobbyAllReadyDetector::process_frame(
    const QImage& frame,
    std::chrono::system_clock::time_point timestamp
){
    return detect(frame);
}
bool LobbyAllReadyDetector::detect(const QImage& screen){
    size_t ready = 0;
    ImageStats stats0 = image_stats(extract_box(screen, m_checkbox0));
    ImageStats stats1 = image_stats(extract_box(screen, m_checkbox1));
    ImageStats stats2 = image_stats(extract_box(screen, m_checkbox2));
    ImageStats stats3 = image_stats(extract_box(screen, m_checkbox3));
    if (stats0.stddev.sum() > 50) ready++;
    if (stats1.stddev.sum() > 50) ready++;
    if (stats2.stddev.sum() > 50) ready++;
    if (stats3.stddev.sum() > 50) ready++;

    return ready >= m_consoles - 1;
}


}
}
}
}
