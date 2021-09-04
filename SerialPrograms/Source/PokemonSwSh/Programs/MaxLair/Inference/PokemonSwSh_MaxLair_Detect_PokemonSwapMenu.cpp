/*  Max Lair Detect Pokemon Swap Menu
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/OCR/Filtering.h"
#include "CommonFramework/OCR/StringNormalization.h"
#include "CommonFramework/OCR/RawOCR.h"
#include "CommonFramework/Inference/ImageTools.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonSwSh/Inference/PokemonSwSh_PokemonSpriteReader.h"
#include "PokemonSwSh/Programs/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PP.h"
#include "PokemonSwSh_MaxLair_Detect_PokemonReader.h"
#include "PokemonSwSh_MaxLair_Detect_PokemonSwapMenu.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


PokemonSwapMenuDetector::PokemonSwapMenuDetector(VideoOverlay& overlay, bool stop_no_detect)
    : m_stop_on_no_detect(stop_no_detect)
    , m_pink0(overlay, 0.10, 0.18, 0.27, 0.12)
    , m_pink1(overlay, 0.15, 0.70, 0.09, 0.15)
    , m_white(overlay, 0.52, 0.13, 0.45, 0.05)
//    , m_bottom(overlay, 0.35, 0.80, 0.10, 0.10)
//    , m_box0(overlay, 0.87, 0.22, 0.03, 0.20)
//    , m_box1(overlay, 0.87, 0.48, 0.03, 0.20)
{}
bool PokemonSwapMenuDetector::on_frame(
    const QImage& frame,
    std::chrono::system_clock::time_point timestamp
){
    return m_stop_on_no_detect
        ? !detect(frame)
        : detect(frame);
}
bool PokemonSwapMenuDetector::detect(const QImage& screen) const{
    ImageStats pink0 = image_stats(extract_box(screen, m_pink0));
    if (!is_solid(pink0, {0.448591, 0.176516, 0.374892}, 0.1, 20)) return false;
    ImageStats pink1 = image_stats(extract_box(screen, m_pink1));
    if (!is_solid(pink1, {0.464402, 0.156018, 0.379581}, 0.1, 20)) return false;
    ImageStats white = image_stats(extract_box(screen, m_white));
//    cout << white.average << ", " << white.stddev << endl;
    if (!is_solid(white, {0.310994, 0.344503, 0.344503})) return false;
//    ImageStats bottom = pixel_stats(extract_box(screen, m_bottom));
//    if (!is_white(bottom)) return false;
//    ImageStats box0 = pixel_stats(extract_box(screen, m_box0));
//    if (!is_white(box0)) return false;
//    ImageStats box1 = pixel_stats(extract_box(screen, m_box1));
//    if (!is_white(box1)) return false;

//    cout << pink0.average << ", " << pink0.stddev << endl;
//    cout << pink1.average << ", " << pink1.stddev << endl;
    return true;
}



PokemonSwapMenuReader::PokemonSwapMenuReader(
    Logger& logger,
    VideoOverlay& overlay,
    Language language
)
    : m_logger(logger)
    , m_language(language)
    , m_sprite0(overlay, 0.485, 0.262 + 0*0.258, 0.063, 0.073)
    , m_sprite1(overlay, 0.485, 0.262 + 1*0.258, 0.063, 0.073)
    , m_name0(overlay, 0.485, 0.335 + 0*0.258, 0.180, 0.045)
    , m_name1(overlay, 0.485, 0.335 + 1*0.258, 0.180, 0.045)
    , m_select0(overlay, 0.630, 0.320 + 0*0.258, 0.030, 0.100)
    , m_select1(overlay, 0.630, 0.320 + 1*0.258, 0.030, 0.100)
    , m_pp0(overlay, 0.900, 0.207 + 0*0.060, 0.070, 0.045)
    , m_pp1(overlay, 0.900, 0.207 + 1*0.060, 0.070, 0.045)
    , m_pp2(overlay, 0.900, 0.207 + 2*0.060, 0.070, 0.045)
    , m_pp3(overlay, 0.900, 0.207 + 3*0.060, 0.070, 0.045)
{}
bool PokemonSwapMenuReader::my_turn(const QImage& screen){
    double box0 = pixel_average(extract_box(screen, m_select0)).sum();
    if (box0 < 200) return true;
    double box1 = pixel_average(extract_box(screen, m_select1)).sum();
    if (box1 < 200) return true;
    return false;
}
void PokemonSwapMenuReader::read_options(const QImage& screen, std::string option[2]){
//    option[0] = read_option(screen, m_sprite0, m_name0);
//    option[1] = read_option(screen, m_sprite1, m_name1);
    option[0] = read_pokemon_name_sprite(m_logger, screen, m_sprite0, m_name0, m_language);
    option[1] = read_pokemon_name_sprite(m_logger, screen, m_sprite1, m_name1, m_language);
}

void PokemonSwapMenuReader::read_pp(const QImage& screen, int8_t pp[4]){
    pp[0] = read_pp_from_swap_menu(m_logger, extract_box(screen, m_pp0));
    pp[1] = read_pp_from_swap_menu(m_logger, extract_box(screen, m_pp1));
    pp[2] = read_pp_from_swap_menu(m_logger, extract_box(screen, m_pp2));
    pp[3] = read_pp_from_swap_menu(m_logger, extract_box(screen, m_pp3));
    if (pp[0] < 0 || pp[1] < 0 || pp[2] < 0 || pp[3] < 0){
        dump_image(m_logger, screen, "PokemonSwapMenuReader-read_pp");
    }
}

























}
}
}
}
