/*  Max Lair Detect Pokemon Swap Menu
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/Notifications/ProgramInfo.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "Pokemon/Inference/Pokemon_ReadHpBar.h"
#include "PokemonSwSh/MaxLair/Options/PokemonSwSh_MaxLair_Options.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_HPPP.h"
#include "PokemonSwSh_MaxLair_Detect_PokemonReader.h"
#include "PokemonSwSh_MaxLair_Detect_PokemonSwapMenu.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


PokemonSwapMenuDetector::PokemonSwapMenuDetector(bool stop_no_detect)
    : VisualInferenceCallback("PokemonSwapMenuDetector")
    , m_stop_on_no_detect(stop_no_detect)
    , m_pink0(0.150, 0.015, 0.270, 0.050)
    , m_pink1(0.100, 0.260, 0.270, 0.040)
    , m_pink2(0.150, 0.700, 0.090, 0.150)
    , m_white0(0.600, 0.015, 0.370, 0.040)
    , m_white1(0.520, 0.130, 0.450, 0.050)
//    , m_bottom(overlay, 0.35, 0.80, 0.10, 0.10)
//    , m_box0(overlay, 0.87, 0.22, 0.03, 0.20)
//    , m_box1(overlay, 0.87, 0.48, 0.03, 0.20)
    , m_bottom_main(0.100, 0.970, 0.600, 0.020)
    , m_bottom_right(0.920, 0.970, 0.070, 0.020)
{}
void PokemonSwapMenuDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_pink0);
    items.add(COLOR_RED, m_pink1);
    items.add(COLOR_RED, m_pink2);
    items.add(COLOR_RED, m_white0);
    items.add(COLOR_RED, m_white1);
    items.add(COLOR_RED, m_bottom_main);
    items.add(COLOR_RED, m_bottom_right);
}
bool PokemonSwapMenuDetector::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    return m_stop_on_no_detect
        ? !detect(frame)
        : detect(frame);
}
bool PokemonSwapMenuDetector::detect(const ImageViewRGB32& screen) const{
    ImageStats pink0 = image_stats(extract_box_reference(screen, m_pink0));
    if (!is_solid(pink0, {0.448591, 0.176516, 0.374892}, 0.1, 20)) return false;
    ImageStats pink1 = image_stats(extract_box_reference(screen, m_pink1));
    if (!is_solid(pink1, {0.448591, 0.176516, 0.374892}, 0.1, 20)) return false;
    ImageStats pink2 = image_stats(extract_box_reference(screen, m_pink2));
    if (!is_solid(pink2, {0.464402, 0.156018, 0.379581}, 0.1, 20)) return false;
    ImageStats white0 = image_stats(extract_box_reference(screen, m_white0));
//    cout << white0.average << ", " << white0.stddev << endl;
    if (!is_solid(white0, {0.318115, 0.33587, 0.346015})) return false;
    ImageStats white1 = image_stats(extract_box_reference(screen, m_white1));
//    cout << white.average << ", " << white.stddev << endl;
    if (!is_solid(white1, {0.310994, 0.344503, 0.344503})) return false;
//    ImageStats bottom = pixel_stats(extract_box(screen, m_bottom));
//    if (!is_white(bottom)) return false;
//    ImageStats box0 = pixel_stats(extract_box(screen, m_box0));
//    if (!is_white(box0)) return false;
//    ImageStats box1 = pixel_stats(extract_box(screen, m_box1));
//    if (!is_white(box1)) return false;

    ImageStats bottom_main = image_stats(extract_box_reference(screen, m_bottom_main));
//    cout << bottom_main.average << ", " << bottom_main.stddev << endl;
    if (!is_black(bottom_main)){
        return false;
    }
    ImageStats bottom_right = image_stats(extract_box_reference(screen, m_bottom_right));
//    cout << bottom_right.average << ", " << bottom_right.stddev << endl;
    if (bottom_right.stddev.sum() < 30){
        return false;
    }

//    cout << pink0.average << ", " << pink0.stddev << endl;
//    cout << pink1.average << ", " << pink1.stddev << endl;
    return true;
}



PokemonSwapMenuReader::PokemonSwapMenuReader(
    Logger& logger,
    VideoOverlay& overlay,
    Language language,
    OcrFailureWatchdog& ocr_watchdog
)
    : m_logger(logger)
    , m_language(language), m_ocr_watchdog(ocr_watchdog)
    , m_sprite0(overlay, {0.481, 0.235 + 0*0.258, 0.071, 0.103})
    , m_sprite1(overlay, {0.481, 0.235 + 1*0.258, 0.071, 0.103})
    , m_name0(overlay, {0.485, 0.335 + 0*0.258, 0.180, 0.045})
    , m_name1(overlay, {0.485, 0.335 + 1*0.258, 0.180, 0.045})
    , m_select0(overlay, {0.630, 0.320 + 0*0.258, 0.030, 0.100})
    , m_select1(overlay, {0.630, 0.320 + 1*0.258, 0.030, 0.100})
    , m_pp0(overlay, {0.900, 0.207 + 0*0.060, 0.070, 0.045})
    , m_pp1(overlay, {0.900, 0.207 + 1*0.060, 0.070, 0.045})
    , m_pp2(overlay, {0.900, 0.207 + 2*0.060, 0.070, 0.045})
    , m_pp3(overlay, {0.900, 0.207 + 3*0.060, 0.070, 0.045})
    , m_hp0(overlay, {0.226, 0.3435 + 0*0.089, 0.112, 0.005})
    , m_hp1(overlay, {0.226, 0.3435 + 1*0.089, 0.112, 0.005})
    , m_hp2(overlay, {0.226, 0.3435 + 2*0.089, 0.112, 0.005})
    , m_hp3(overlay, {0.226, 0.3435 + 3*0.089, 0.112, 0.005})
{}
bool PokemonSwapMenuReader::my_turn(const ImageViewRGB32& screen){
    double box0 = image_average(extract_box_reference(screen, m_select0)).sum();
    if (box0 < 200) return true;
    double box1 = image_average(extract_box_reference(screen, m_select1)).sum();
    if (box1 < 200) return true;
    return false;
}
void PokemonSwapMenuReader::read_options(const ImageViewRGB32& screen, std::string option[2]){
    option[0] = read_pokemon_name_sprite(m_logger, m_ocr_watchdog, screen, m_sprite0, m_name0, m_language, true);
    option[1] = read_pokemon_name_sprite(m_logger, m_ocr_watchdog, screen, m_sprite1, m_name1, m_language, true);
}

void PokemonSwapMenuReader::read_hp(const ImageViewRGB32& screen, double hp[4]){
    hp[0] = read_hp_bar(m_logger, extract_box_reference(screen, m_hp0));
    hp[1] = read_hp_bar(m_logger, extract_box_reference(screen, m_hp1));
    hp[2] = read_hp_bar(m_logger, extract_box_reference(screen, m_hp2));
    hp[3] = read_hp_bar(m_logger, extract_box_reference(screen, m_hp3));
    if (hp[0] < 0 || hp[1] < 0 || hp[2] < 0 || hp[3] < 0){
        dump_image(m_logger, MODULE_NAME, "PokemonSwapMenuReader-read_hp", screen);
    }
}
void PokemonSwapMenuReader::read_pp(const ImageViewRGB32& screen, int8_t pp[4]){
    pp[0] = read_pp_text(m_logger, extract_box_reference(screen, m_pp0));
    pp[1] = read_pp_text(m_logger, extract_box_reference(screen, m_pp1));
    pp[2] = read_pp_text(m_logger, extract_box_reference(screen, m_pp2));
    pp[3] = read_pp_text(m_logger, extract_box_reference(screen, m_pp3));
    if (pp[0] < 0 && pp[1] < 0 && pp[2] < 0 && pp[3] < 0){
        dump_image(m_logger, MODULE_NAME, "PokemonSwapMenuReader-read_pp", screen);
        return;
    }
}

























}
}
}
}
