/*  Max Lair Detect Path Select
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Compiler.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "PokemonSwSh/Programs/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PokemonReader.h"
#include "PokemonSwSh_MaxLair_Detect_PathSelect.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


PathSelectDetector::PathSelectDetector(VideoOverlay& overlay)
    : m_bottom_main(overlay, 0.100, 0.970, 0.700, 0.020, Qt::cyan)
    , m_bottom_right(overlay, 0.920, 0.970, 0.070, 0.020, Qt::cyan)
    , m_dialog_left(overlay, 0.257, 0.807, 0.015, 0.030, Qt::cyan)
    , m_dialog_middle(overlay, 0.500, 0.880, 0.180, 0.050, Qt::cyan)
    , m_dialog_right(overlay, 0.710, 0.880, 0.030, 0.050, Qt::cyan)
{}


bool PathSelectDetector::detect(const QImage& screen) const{
    ImageStats bottom_main = image_stats(extract_box(screen, m_bottom_main));
    if (!is_black(bottom_main)){
        return false;
    }
    ImageStats bottom_right = image_stats(extract_box(screen, m_bottom_right));
    if (bottom_right.stddev.sum() < 30){
        return false;
    }
    ImageStats dialog_left = image_stats(extract_box(screen, m_dialog_left));
    if (!is_grey(dialog_left, 0, 200)){
        return false;
    }
    ImageStats dialog_middle = image_stats(extract_box(screen, m_dialog_middle));
    if (!is_grey(dialog_middle, 0, 200)){
        return false;
    }
    ImageStats dialog_right = image_stats(extract_box(screen, m_dialog_right));
    if (!is_grey(dialog_right, 0, 200)){
        return false;
    }
    if (euclidean_distance(dialog_left.average, dialog_right.average) > 10){
        return false;
    }
    if (dialog_left.average.sum() > dialog_middle.average.sum()){
        return false;
    }
    if (dialog_right.average.sum() > dialog_middle.average.sum()){
        return false;
    }
    return true;
}
bool PathSelectDetector::on_frame(
    const QImage& frame,
    std::chrono::system_clock::time_point timestamp
){
    return detect(frame);
}




PathPartyReader::PathPartyReader(VideoOverlay& overlay)
    : m_sprite0(overlay, 0.002, 0.347 + 0*0.163, 0.071 - 0.005, 0.102)
    , m_sprite1(overlay, 0.002, 0.347 + 1*0.163, 0.071 - 0.005, 0.102)
    , m_sprite2(overlay, 0.002, 0.347 + 2*0.163, 0.071 - 0.005, 0.102)
    , m_sprite3(overlay, 0.002, 0.347 + 3*0.163, 0.071 - 0.005, 0.102)
{}

void PathPartyReader::read_sprites(Logger& logger, const QImage& screen, std::string slugs[4]){
    slugs[0] = read_pokemon_sprite(logger, extract_box(screen, m_sprite0));
    slugs[1] = read_pokemon_sprite(logger, extract_box(screen, m_sprite1));
    slugs[2] = read_pokemon_sprite(logger, extract_box(screen, m_sprite2));
    slugs[3] = read_pokemon_sprite(logger, extract_box(screen, m_sprite3));
    if (slugs[0].empty() || slugs[1].empty() || slugs[2].empty() || slugs[3].empty()){
        dump_image(logger, screen, "PathPartyReader-ReadSprites");
    }
}








}
}
}
}
