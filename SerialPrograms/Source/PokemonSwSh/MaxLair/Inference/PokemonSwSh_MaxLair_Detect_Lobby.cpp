/*  Max Lair Detect Lobby
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTools/ImageDiff.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "PokemonSwSh_MaxLair_Detect_Lobby.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{



#if 1
LobbyDetector::LobbyDetector(bool invert)
    : VisualInferenceCallback("LobbyDetector")
    , m_invert(invert)
    , m_pink (0.575, 0.035, 0.050, 0.100)
    , m_white(0.800, 0.200, 0.150, 0.100)
{}
void LobbyDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_pink);
    items.add(COLOR_RED, m_white);
}
bool LobbyDetector::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    return detect(frame);
}
bool LobbyDetector::detect(const ImageViewRGB32& screen){
    ImageStats stats0 = image_stats(extract_box_reference(screen, m_pink));
    ImageStats stats1 = image_stats(extract_box_reference(screen, m_white));
//    cout << stats0.average << ", " << stats0.stddev << endl;
//    cout << stats1.average << ", " << stats1.stddev << endl;
    if (!is_solid(stats0, {0.444944, 0.150562, 0.404494})){
        return m_invert;
    }
    if (!is_solid(stats1, {0.303079, 0.356564, 0.340357})){
        return m_invert;
    }
    return !m_invert;
}
#endif


LobbyDoneConnecting::LobbyDoneConnecting()
    : VisualInferenceCallback("LobbyDoneConnecting")
    , m_box(0.600, 0.820, 0.080, 0.100)
    , m_player0(0.669, 0.337 + 0.0775*1, 0.100, 0.06)
{}
void LobbyDoneConnecting::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_box);
    items.add(COLOR_RED, m_player0);
}
bool LobbyDoneConnecting::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    return detect(frame);
}
bool LobbyDoneConnecting::detect(const ImageViewRGB32& screen){
    ImageStats stats0 = image_stats(extract_box_reference(screen, m_box));
//    cout << stats0.average << ", " << stats0.stddev << endl;
    if (is_grey(stats0, 0, 200)){
        return false;
    }
    ImageStats player0 = image_stats(extract_box_reference(screen, m_player0));
//    cout << player0.average << player0.stddev << endl;
    if (is_white(player0, 300, 20)){
        return false;
    }
//    screen.save("test.png");
    return true;
}



LobbyJoinedDetector::LobbyJoinedDetector(size_t consoles, bool invert)
    : VisualInferenceCallback("LobbyJoinedDetector")
    , m_consoles(consoles)
    , m_invert(invert)
    , m_box0(0.705, 0.337 + 0.0775*0, 0.034, 0.06)
    , m_box1(0.705, 0.337 + 0.0775*1, 0.034, 0.06)
    , m_box2(0.705, 0.337 + 0.0775*2, 0.034, 0.06)
    , m_box3(0.705, 0.337 + 0.0775*3, 0.034, 0.06)
    , m_player0(COLOR_CYAN, {0, 0, 1, 0.5}, std::chrono::seconds(1), 10)
    , m_player1(COLOR_CYAN, {0, 0, 1, 0.5}, std::chrono::seconds(1), 10)
    , m_player2(COLOR_CYAN, {0, 0, 1, 0.5}, std::chrono::seconds(1), 10)
    , m_player3(COLOR_CYAN, {0, 0, 1, 0.5}, std::chrono::seconds(1), 10)
{}
void LobbyJoinedDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_box0);
    items.add(COLOR_RED, m_box1);
    items.add(COLOR_RED, m_box2);
    items.add(COLOR_RED, m_box3);
    m_player0.make_overlays(items);
    m_player1.make_overlays(items);
    m_player2.make_overlays(items);
    m_player3.make_overlays(items);
}

size_t LobbyJoinedDetector::joined(const ImageViewRGB32& screen, WallClock timestamp){
    size_t count = 0;
    if (m_player0.process_frame(extract_box_reference(screen, m_box0), timestamp)) count++;
    if (m_player1.process_frame(extract_box_reference(screen, m_box1), timestamp)) count++;
    if (m_player2.process_frame(extract_box_reference(screen, m_box2), timestamp)) count++;
    if (m_player3.process_frame(extract_box_reference(screen, m_box3), timestamp)) count++;
    return count;
}

bool LobbyJoinedDetector::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    return m_invert
        ? joined(frame, timestamp) < m_consoles
        : joined(frame, timestamp) >= m_consoles;
}




LobbyReadyDetector::LobbyReadyDetector()
    : VisualInferenceCallback("LobbyReadyDetector")
    , m_checkbox0(0.669, 0.337 + 0.0775*0, 0.034, 0.06)
    , m_checkbox1(0.669, 0.337 + 0.0775*1, 0.034, 0.06)
    , m_checkbox2(0.669, 0.337 + 0.0775*2, 0.034, 0.06)
    , m_checkbox3(0.669, 0.337 + 0.0775*3, 0.034, 0.06)
{}
void LobbyReadyDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_checkbox0);
    items.add(COLOR_RED, m_checkbox1);
    items.add(COLOR_RED, m_checkbox2);
    items.add(COLOR_RED, m_checkbox3);
}
size_t LobbyReadyDetector::ready_players(const ImageViewRGB32& screen){
    size_t ready = 0;
    ImageStats stats0 = image_stats(extract_box_reference(screen, m_checkbox0));
    ImageStats stats1 = image_stats(extract_box_reference(screen, m_checkbox1));
    ImageStats stats2 = image_stats(extract_box_reference(screen, m_checkbox2));
    ImageStats stats3 = image_stats(extract_box_reference(screen, m_checkbox3));
    if (stats0.stddev.sum() > 50) ready++;
    if (stats1.stddev.sum() > 50) ready++;
    if (stats2.stddev.sum() > 50) ready++;
    if (stats3.stddev.sum() > 50) ready++;
    return ready;
}
bool LobbyReadyDetector::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    return detect(frame);
}

LobbyMinReadyDetector::LobbyMinReadyDetector(size_t consoles, bool invert)
    : m_consoles(consoles)
    , m_invert(invert)
{}
bool LobbyMinReadyDetector::detect(const ImageViewRGB32& screen){
    return m_invert
        ? ready_players(screen) < m_consoles
        : ready_players(screen) >= m_consoles;
}

LobbyAllReadyDetector::LobbyAllReadyDetector(size_t consoles)
    : m_consoles(consoles)
{}
bool LobbyAllReadyDetector::detect(const ImageViewRGB32& screen){
    return ready_players(screen) >= m_consoles;
}


}
}
}
}
