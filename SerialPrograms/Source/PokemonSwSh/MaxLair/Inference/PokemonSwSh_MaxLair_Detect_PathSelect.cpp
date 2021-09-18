/*  Max Lair Detect Path Select
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Compiler.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "CommonFramework/Inference/ImageTools.h"
#include "PokemonSwSh/Inference/PokemonSwSh_TypeSymbolFinder.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PokemonReader.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_HPPP.h"
#include "PokemonSwSh_MaxLair_Detect_PathSelect.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


PathSelectDetector::PathSelectDetector()
    : m_bottom_main(0.100, 0.970, 0.700, 0.020)
    , m_bottom_right(0.920, 0.970, 0.070, 0.020)
    , m_dialog_left(0.257, 0.807, 0.015, 0.030)
    , m_dialog_middle(0.500, 0.880, 0.180, 0.050)
//    , m_dialog_right(0.710, 0.880, 0.030, 0.050)
    , m_main(0.100, 0.100, 0.800, 0.600)
    , m_left(0.050, 0.100, 0.200, 0.700)
    , m_box0(0.074, 0.420 + 0*0.16315, 0.020, 0.007)
    , m_box1(0.074, 0.420 + 1*0.16315, 0.020, 0.007)
    , m_box2(0.074, 0.420 + 2*0.16315, 0.020, 0.007)
    , m_box3(0.074, 0.420 + 3*0.16315, 0.020, 0.007)
{
    add_box(m_bottom_main, Qt::cyan);
    add_box(m_bottom_right, Qt::cyan);
    add_box(m_dialog_left, Qt::cyan);
    add_box(m_dialog_middle, Qt::cyan);
//    add_box(m_dialog_right, Qt::cyan);
    add_box(m_main, Qt::cyan);
    add_box(m_left, Qt::cyan);
    add_box(m_box0, Qt::cyan);
    add_box(m_box1, Qt::cyan);
    add_box(m_box2, Qt::cyan);
    add_box(m_box3, Qt::cyan);
}


bool PathSelectDetector::detect(const QImage& screen) const{
    ImageStats box0 = image_stats(extract_box(screen, m_box0));
    if (!is_white(box0, 500, 40)){
//        global_logger().log("box0 out");
        return false;
    }
    ImageStats box1 = image_stats(extract_box(screen, m_box1));
    if (!is_white(box1, 500, 40)){
//        global_logger().log("box1 out");
//        cout << "box1 = " << box1.average << box1.stddev << endl;
        return false;
    }
    ImageStats box2 = image_stats(extract_box(screen, m_box2));
    if (!is_white(box2, 500, 40)){
//        global_logger().log("box2 out");
//        cout << "box2 = " << box2.average << box2.stddev << endl;
        return false;
    }
    ImageStats box3 = image_stats(extract_box(screen, m_box3));
    if (!is_white(box3, 500, 40)){
//        global_logger().log("box3 out");
//        cout << "box3 = " << box3.average << box3.stddev << endl;
        return false;
    }

    ImageStats bottom_main = image_stats(extract_box(screen, m_bottom_main));
    if (!is_black(bottom_main)){
//        global_logger().log("m_bottom_main out");
        return false;
    }
    ImageStats bottom_right = image_stats(extract_box(screen, m_bottom_right));
    if (bottom_right.stddev.sum() < 30){
//        cout << "m_bottom_right = " << bottom_right.average << bottom_right.stddev << endl;
//        global_logger().log("m_bottom_right out");
        return false;
    }
    ImageStats dialog_left = image_stats(extract_box(screen, m_dialog_left));
    if (!is_grey(dialog_left, 0, 200)){
//        global_logger().log("m_dialog_left out");
        return false;
    }
    ImageStats dialog_middle = image_stats(extract_box(screen, m_dialog_middle));
    if (!is_grey(dialog_middle, 0, 200)){
//        global_logger().log("m_dialog_middle out");
        return false;
    }
//    ImageStats dialog_right = image_stats(extract_box(screen, m_dialog_right));
//    if (!is_grey(dialog_right, 0, 200)){
//        return false;
//    }
    ImageStats main = image_stats(extract_box(screen, m_main));
//    cout << main.average << main.stddev << endl;
    if (main.stddev.sum() < 50){
//        global_logger().log("m_main out");
        return false;
    }
    ImageStats left = image_stats(extract_box(screen, m_left));
//    cout << left.average << left.stddev << endl;
    if (left.stddev.sum() < 100){
//        global_logger().log("m_left out");
        return false;
    }
//    if (euclidean_distance(dialog_left.average, dialog_right.average) > 10){
//        return false;
//    }
    if (dialog_left.average.sum() > dialog_middle.average.sum()){
        global_logger().log("dialog out");
        return false;
    }
//    if (dialog_right.average.sum() > dialog_middle.average.sum()){
//        return false;
//    }
    return true;
}
bool PathSelectDetector::process_frame(
    const QImage& frame,
    std::chrono::system_clock::time_point timestamp
){
    bool ret = detect(frame);
//    global_logger().log(std::to_string(ret));
    return ret;
}




bool read_type_array(
    ConsoleHandle& console,
    const ImageFloatBox& box,
    std::deque<InferenceBoxScope>& hits,
    size_t count,
    PokemonType* type, ImagePixelBox* boxes
){
    QImage screen = console.video().snapshot();
    QImage image = extract_box(screen, box);
    for (size_t c = 0; c < count; c++){
        type[c] = PokemonType::NONE;
    }

    std::multimap<double, std::pair<PokemonType, ImagePixelBox>> candidates = find_symbols(image, 0.20);
    if (candidates.size() < count){
        return false;
    }

    std::multimap<pxint_t, const std::pair<PokemonType, ImagePixelBox>*> sorted;
//    std::deque<InferenceBoxScope> hits;
    hits.clear();
    size_t c = 0;
    for (const auto& item : candidates){
        hits.emplace_back(console.overlay(), translate_to_parent(screen, box, item.second.second), Qt::green);
        sorted.emplace(item.second.second.min_x, &item.second);
        c++;
        if (c >= count){
            break;
        }
    }

    c = 0;
    for (const auto& item : sorted){
        type[c] = item.second->first;
        if (boxes != nullptr){
            boxes[c] = item.second->second;
        }
        c++;
    }

    return true;
}
bool read_path(Path& path, ConsoleHandle& console){
    InferenceBoxScope box(console.overlay(), 0.150, 0.200, 0.800, 0.600);
    bool ok = true;
    console.botbase().wait_for_all_requests();

    std::deque<InferenceBoxScope> hits;

    ok &= read_type_array(console, box, hits, 2, path.mon1, nullptr);
    pbf_move_right_joystick(console, 128, 0, 75, 50);
    console.botbase().wait_for_all_requests();

    ok &= read_type_array(console, box, hits, 4, path.mon2, nullptr);
    pbf_move_right_joystick(console, 128, 0, 110, 50);
    console.botbase().wait_for_all_requests();

    ok &= read_type_array(console, box, hits, 4, path.mon3, nullptr);

    return ok;
}







PathPartyReader::PathPartyReader(VideoOverlay& overlay, size_t player_index)
    : m_player_index(player_index)
    , m_sprite0(overlay, 0.002, 0.345 + 0*0.16315, 0.071, 0.102)
    , m_sprite1(overlay, 0.002, 0.345 + 1*0.16315, 0.071, 0.102)
    , m_sprite2(overlay, 0.002, 0.345 + 2*0.16315, 0.071, 0.102)
    , m_sprite3(overlay, 0.002, 0.345 + 3*0.16315, 0.071, 0.102)
    , m_hp0(overlay, 0.074, 0.435 + 0*0.16315, player_index == 0 ? 0.052 : 0.041, 0.007)
    , m_hp1(overlay, 0.074, 0.435 + 1*0.16315, player_index == 1 ? 0.052 : 0.041, 0.007)
    , m_hp2(overlay, 0.074, 0.435 + 2*0.16315, player_index == 2 ? 0.052 : 0.041, 0.007)
    , m_hp3(overlay, 0.074, 0.435 + 3*0.16315, player_index == 3 ? 0.052 : 0.041, 0.007)
{}

void PathPartyReader::read_sprites(Logger& logger, const QImage& screen, std::string slugs[4]) const{
    slugs[0] = read_pokemon_sprite_with_item(logger, screen, m_sprite0);
    slugs[1] = read_pokemon_sprite_with_item(logger, screen, m_sprite1);
    slugs[2] = read_pokemon_sprite_with_item(logger, screen, m_sprite2);
    slugs[3] = read_pokemon_sprite_with_item(logger, screen, m_sprite3);
    if (slugs[0].empty() || slugs[1].empty() || slugs[2].empty() || slugs[3].empty()){
        dump_image(logger, screen, "PathPartyReader-ReadSprites");
    }
}
void PathPartyReader::read_hp(Logger& logger, const QImage& screen, double hp[4]) const{
    hp[0] = read_hp_bar(logger, extract_box(screen, m_hp0));
    hp[1] = read_hp_bar(logger, extract_box(screen, m_hp1));
    hp[2] = read_hp_bar(logger, extract_box(screen, m_hp2));
    hp[3] = read_hp_bar(logger, extract_box(screen, m_hp3));
    if (hp[0] < 0 || hp[1] < 0 || hp[2] < 0 || hp[3] < 0){
        dump_image(logger, screen, "PathPartyReader-ReadHP");
    }
}


void PathPartyReader::read_sprites(
    Logger& logger,
    ConsoleHandle& console,
    GlobalState& state,
    const QImage& screen
) const{
    std::string mons[4];
    read_sprites(console, console.video().snapshot(), mons);
    if (!mons[0].empty()) state.players[0].pokemon = mons[0];
    if (!mons[1].empty()) state.players[1].pokemon = mons[1];
    if (!mons[2].empty()) state.players[2].pokemon = mons[2];
    if (!mons[3].empty()) state.players[3].pokemon = mons[3];
}
void PathPartyReader::read_hp(
    Logger& logger,
    ConsoleHandle& console,
    GlobalState& state,
    const QImage& screen
) const{
    double hp[4];
    read_hp(console, console.video().snapshot(), hp);
    if (hp[0] >= 0) state.players[0].health.value.hp = hp[0];
    if (hp[1] >= 0) state.players[1].health.value.hp = hp[1];
    if (hp[2] >= 0) state.players[2].health.value.hp = hp[2];
    if (hp[3] >= 0) state.players[3].health.value.hp = hp[3];
    state.players[0].health.value.dead = 0;
    state.players[1].health.value.dead = 0;
    state.players[2].health.value.dead = 0;
    state.players[3].health.value.dead = 0;
}







}
}
}
}
