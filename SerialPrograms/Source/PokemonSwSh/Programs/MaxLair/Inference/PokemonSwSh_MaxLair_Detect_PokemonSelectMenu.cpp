/*  Max Lair Detect Pokemon Select Menu
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/OCR/Filtering.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonSwSh_MaxLair_Detect_PokemonReader.h"
#include "PokemonSwSh_MaxLair_Detect_PokemonSelectMenu.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


PokemonSelectMenuDetector::PokemonSelectMenuDetector(VideoOverlay& overlay, bool stop_no_detect)
    : m_stop_on_no_detect(stop_no_detect)
    , m_box0(overlay, 0.02, 0.02, 0.40, 0.04)
//    , m_box1(overlay, 0.09, 0.18, 0.30, 0.10)
    , m_box1(overlay, 0.10, 0.18, 0.27, 0.12)
//    , m_box2(overlay, 0.15, 0.72, 0.10, 0.10)
//    , m_box3(overlay, 0.35, 0.80, 0.10, 0.10)
//    , m_box4(overlay, 0.87, 0.17, 0.03, 0.20)
//    , m_box5(overlay, 0.87, 0.43, 0.03, 0.20)
//    , m_box6(overlay, 0.87, 0.69, 0.03, 0.20)
{}
bool PokemonSelectMenuDetector::is_pink(const ImageStats& stats){
    if (stats.average.sum() < 400){
        return false;
    }
    return is_solid(stats, {0.448935, 0.176565, 0.3745});
}
bool PokemonSelectMenuDetector::detect(const QImage& screen) const{
    ImageStats box0 = image_stats(extract_box(screen, m_box0));
    if (!is_pink(box0)) return false;
    ImageStats box1 = image_stats(extract_box(screen, m_box1));
    if (!is_pink(box1)) return false;
//    ImageStats box2 = pixel_stats(extract_box(screen, m_box2));
//    if (!is_pink(box2)) return false;
//    ImageStats box3 = pixel_stats(extract_box(screen, m_box3));
//    if (!is_white(box3)) return false;
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

    return true;
}
bool PokemonSelectMenuDetector::on_frame(
    const QImage& frame,
    std::chrono::system_clock::time_point timestamp
){
    return m_stop_on_no_detect
        ? !detect(frame)
        : detect(frame);
}


PokemonSelectMenuReader::PokemonSelectMenuReader(
    Logger& logger,
    VideoOverlay& overlay,
    Language language
)
    : m_logger(logger)
    , m_language(language)
    , m_sprite0(overlay, 0.485, 0.212 + 0*0.258, 0.063, 0.073)
    , m_sprite1(overlay, 0.485, 0.212 + 1*0.258, 0.063, 0.073)
    , m_sprite2(overlay, 0.485, 0.212 + 2*0.258, 0.063, 0.073)
    , m_name0(overlay, 0.485, 0.285 + 0*0.258, 0.180, 0.045)
    , m_name1(overlay, 0.485, 0.285 + 1*0.258, 0.180, 0.045)
    , m_name2(overlay, 0.485, 0.285 + 2*0.258, 0.180, 0.045)
    , m_select0(overlay, 0.630, 0.270 + 0*0.258, 0.030, 0.100)
    , m_select1(overlay, 0.630, 0.270 + 1*0.258, 0.030, 0.100)
    , m_select2(overlay, 0.630, 0.270 + 2*0.258, 0.030, 0.100)
    , m_player0(overlay, 0.200, 0.335 + 0*0.090, 0.200, 0.060)
    , m_player1(overlay, 0.200, 0.335 + 1*0.090, 0.200, 0.060)
    , m_player2(overlay, 0.200, 0.335 + 2*0.090, 0.200, 0.060)
    , m_player3(overlay, 0.200, 0.335 + 3*0.090, 0.200, 0.060)
{}
bool PokemonSelectMenuReader::my_turn(const QImage& screen) const{
    double box0 = pixel_average(extract_box(screen, m_select0)).sum();
    if (box0 < 200) return true;
    double box1 = pixel_average(extract_box(screen, m_select1)).sum();
    if (box1 < 200) return true;
    double box2 = pixel_average(extract_box(screen, m_select2)).sum();
    if (box2 < 200) return true;
    return false;
}
int8_t PokemonSelectMenuReader::who_is_selecting(const QImage& screen) const{
//    cout << slot0.average << ", " << slot0.stddev << endl;
    ImageStats slot3 = image_stats(extract_box(screen, m_player3));
    if (slot3.stddev.sum() > 30) return 3;
    ImageStats slot2 = image_stats(extract_box(screen, m_player2));
    if (slot2.stddev.sum() > 30) return 2;
    ImageStats slot1 = image_stats(extract_box(screen, m_player1));
    if (slot1.stddev.sum() > 30) return 1;
    ImageStats slot0 = image_stats(extract_box(screen, m_player0));
    if (slot0.stddev.sum() > 30) return 0;
    return -1;
}
std::string PokemonSelectMenuReader::read_option(const QImage& screen, size_t index){
    switch (index){
    case 0: return read_pokemon_name_sprite(m_logger, screen, m_sprite0, m_name0, m_language);
    case 1: return read_pokemon_name_sprite(m_logger, screen, m_sprite1, m_name1, m_language);
    case 2: return read_pokemon_name_sprite(m_logger, screen, m_sprite2, m_name2, m_language);
    }
    return "";
}
#if 0
void PokemonSelectMenuReader::read_options(const QImage& screen, std::string option[3]){
    option[0] = read_name_sprite(m_logger, screen, m_sprite0, m_name0, m_language);
//    option[1] = read_name_sprite(m_logger, screen, m_sprite1, m_name1, m_language);
//    option[2] = read_name_sprite(m_logger, screen, m_sprite2, m_name2, m_language);
}
#endif










}
}
}
}
