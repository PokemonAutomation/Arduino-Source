/*  Max Lair Detect Pokemon Select Menu
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "PokemonSwSh_MaxLair_Detect_PokemonReader.h"
#include "PokemonSwSh_MaxLair_Detect_PokemonSelectMenu.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


PokemonSelectMenuDetector::PokemonSelectMenuDetector(bool stop_no_detect)
    : VisualInferenceCallback("PokemonSelectMenuDetector")
    , m_stop_on_no_detect(stop_no_detect)
    , m_box0(0.02, 0.02, 0.40, 0.04)
//    , m_box1(overlay, 0.09, 0.18, 0.30, 0.10)
    , m_box1(0.10, 0.18, 0.27, 0.12)
    , m_box2(0.50, 0.02, 0.15, 0.03)
    , m_box3(0.55, 0.07, 0.10, 0.05)
//    , m_box4(overlay, 0.87, 0.17, 0.03, 0.20)
//    , m_box5(overlay, 0.87, 0.43, 0.03, 0.20)
//    , m_box6(overlay, 0.87, 0.69, 0.03, 0.20)
    , m_select0(0.630, 0.270 + 0*0.258, 0.030, 0.100)
    , m_select1(0.630, 0.270 + 1*0.258, 0.030, 0.100)
    , m_select2(0.630, 0.270 + 2*0.258, 0.030, 0.100)
{}
void PokemonSelectMenuDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_box0);
    items.add(COLOR_RED, m_box1);
    items.add(COLOR_RED, m_box2);
    items.add(COLOR_RED, m_box3);
    items.add(COLOR_RED, m_select0);
    items.add(COLOR_RED, m_select1);
    items.add(COLOR_RED, m_select2);
}
bool PokemonSelectMenuDetector::is_pink(const ImageStats& stats){
    if (stats.average.sum() < 400){
        return false;
    }
    return is_solid(stats, {0.448935, 0.176565, 0.3745});
}
bool PokemonSelectMenuDetector::detect(const ImageViewRGB32& screen) const{
    ImageStats box0 = image_stats(extract_box_reference(screen, m_box0));
    if (!is_pink(box0)) return false;
    ImageStats box1 = image_stats(extract_box_reference(screen, m_box1));
    if (!is_pink(box1)) return false;
    ImageStats box2 = image_stats(extract_box_reference(screen, m_box2));
    if (!is_solid(box2, {0.53822, 0.077108, 0.384672})) return false;
    ImageStats box3 = image_stats(extract_box_reference(screen, m_box3));
//    cout << box3.average << box3.stddev << endl;
    if (!is_solid(box3, {0.316905, 0.339245, 0.34385})) return false;
//    ImageStats box4 = pixel_stats(extract_box(screen, m_box4));
//    if (!is_white(box4)) return false;
//    ImageStats box5 = pixel_stats(extract_box(screen, m_box5));
//    if (!is_white(box5)) return false;
//    ImageStats box6 = pixel_stats(extract_box(screen, m_box6));
//    if (!is_white(box6)) return false;

    if (euclidean_distance(box0.average, box1.average) > 10) return false;
//    if (euclidean_distance(box0.average, box2.average) > 10) return false;
//    if (euclidean_distance(box3.average, box4.average) > 10) return false;
//    if (euclidean_distance(box3.average, box5.average) > 10) return false;
///    if (euclidean_distance(box3.average, box6.average) > 10) return false;

    double select0 = image_average(extract_box_reference(screen, m_select0)).sum();
    if (select0 < 200) return true;
//    double select1 = image_average(extract_box(screen, m_select1)).sum();
//    if (select1 < 200) return true;
//    double select2 = image_average(extract_box(screen, m_select2)).sum();
//    if (select2 < 200) return true;
    return false;
}
bool PokemonSelectMenuDetector::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    return m_stop_on_no_detect
        ? !detect(frame)
        : detect(frame);
}


PokemonSelectMenuReader::PokemonSelectMenuReader(
    Logger& logger,
    VideoOverlay& overlay,
    Language language,
    OcrFailureWatchdog& ocr_watchdog
)
    : m_logger(logger)
    , m_language(language)
    , m_ocr_watchdog(ocr_watchdog)
    , m_sprite0(overlay, {0.481, 0.178 + 0*0.258, 0.071, 0.103})
    , m_sprite1(overlay, {0.481, 0.178 + 1*0.258, 0.071, 0.103})
    , m_sprite2(overlay, {0.481, 0.178 + 2*0.258, 0.071, 0.103})
    , m_name0(overlay, {0.485, 0.285 + 0*0.258, 0.180, 0.045})
    , m_name1(overlay, {0.485, 0.285 + 1*0.258, 0.180, 0.045})
    , m_name2(overlay, {0.485, 0.285 + 2*0.258, 0.180, 0.045})
    , m_player0(overlay, {0.200, 0.335 + 0*0.090, 0.200, 0.060})
    , m_player1(overlay, {0.200, 0.335 + 1*0.090, 0.200, 0.060})
    , m_player2(overlay, {0.200, 0.335 + 2*0.090, 0.200, 0.060})
    , m_player3(overlay, {0.200, 0.335 + 3*0.090, 0.200, 0.060})
{}
int8_t PokemonSelectMenuReader::who_is_selecting(const ImageViewRGB32& screen) const{
//    cout << slot0.average << ", " << slot0.stddev << endl;
    ImageStats slot3 = image_stats(extract_box_reference(screen, m_player3));
    if (slot3.stddev.sum() > 30) return 3;
    ImageStats slot2 = image_stats(extract_box_reference(screen, m_player2));
    if (slot2.stddev.sum() > 30) return 2;
    ImageStats slot1 = image_stats(extract_box_reference(screen, m_player1));
    if (slot1.stddev.sum() > 30) return 1;
    ImageStats slot0 = image_stats(extract_box_reference(screen, m_player0));
    if (slot0.stddev.sum() > 30) return 0;
    return -1;
}
std::string PokemonSelectMenuReader::read_option(const ImageViewRGB32& screen, size_t index){
    switch (index){
    case 0: return read_pokemon_name_sprite(m_logger, m_ocr_watchdog, screen, m_sprite0, m_name0, m_language, true);
    case 1: return read_pokemon_name_sprite(m_logger, m_ocr_watchdog, screen, m_sprite1, m_name1, m_language, true);
    case 2: return read_pokemon_name_sprite(m_logger, m_ocr_watchdog, screen, m_sprite2, m_name2, m_language, true);
    }
    return "";
}










}
}
}
}
