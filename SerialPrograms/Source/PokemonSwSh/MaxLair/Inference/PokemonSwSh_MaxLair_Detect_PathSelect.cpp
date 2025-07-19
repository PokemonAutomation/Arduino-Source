/*  Max Lair Detect Path Select
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Notifications/ProgramInfo.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "Pokemon/Inference/Pokemon_ReadHpBar.h"
#include "PokemonSwSh/MaxLair/Options/PokemonSwSh_MaxLair_Options.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_PokemonReader.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_HPPP.h"
#include "PokemonSwSh_MaxLair_Detect_PathSide.h"
#include "PokemonSwSh_MaxLair_Detect_PathMap.h"
#include "PokemonSwSh_MaxLair_Detect_PathSelect.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


PathScreenDetector::PathScreenDetector()
    : VisualInferenceCallback("PathScreenDetector")
    , m_bottom_main(0.100, 0.970, 0.600, 0.020)
    , m_main(0.100, 0.100, 0.800, 0.600)
    , m_box0(0.074, 0.420 + 0*0.16315, 0.020, 0.007)
    , m_box1(0.074, 0.420 + 1*0.16315, 0.020, 0.007)
    , m_box2(0.074, 0.420 + 2*0.16315, 0.020, 0.007)
    , m_box3(0.074, 0.420 + 3*0.16315, 0.020, 0.007)
{}
void PathScreenDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_CYAN, m_bottom_main);
    items.add(COLOR_CYAN, m_main);
    items.add(COLOR_CYAN, m_box0);
    items.add(COLOR_CYAN, m_box1);
    items.add(COLOR_CYAN, m_box2);
    items.add(COLOR_CYAN, m_box3);
}

bool PathScreenDetector::detect(const ImageViewRGB32& screen) const{
    ImageStats box0 = image_stats(extract_box_reference(screen, m_box0));
    if (!is_white(box0, 500, 40)){
//        global_logger().log("box0 out");
        return false;
    }
    ImageStats box1 = image_stats(extract_box_reference(screen, m_box1));
    if (!is_white(box1, 500, 40)){
//        global_logger().log("box1 out");
//        cout << "box1 = " << box1.average << box1.stddev << endl;
        return false;
    }
    ImageStats box2 = image_stats(extract_box_reference(screen, m_box2));
    if (!is_white(box2, 500, 40)){
//        global_logger().log("box2 out");
//        cout << "box2 = " << box2.average << box2.stddev << endl;
        return false;
    }
    ImageStats box3 = image_stats(extract_box_reference(screen, m_box3));
    if (!is_white(box3, 500, 40)){
//        global_logger().log("box3 out");
//        cout << "box3 = " << box3.average << box3.stddev << endl;
        return false;
    }

    ImageStats bottom_main = image_stats(extract_box_reference(screen, m_bottom_main));
    if (!is_black(bottom_main)){
//        global_logger().log("m_bottom_main out");
        return false;
    }
    ImageStats main = image_stats(extract_box_reference(screen, m_main));
//    cout << main.average << main.stddev << endl;
    if (main.stddev.sum() < 50){
//        global_logger().log("m_main out");
        return false;
    }
    return true;
}
bool PathScreenDetector::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    bool ret = detect(frame);
//    global_logger().log(std::to_string(ret));
    return ret;
}





PathSelectDetector::PathSelectDetector()
    : m_bottom_right(0.920, 0.970, 0.070, 0.020)
    , m_dialog_left(0.257, 0.807, 0.015, 0.030)
    , m_dialog_middle(0.500, 0.880, 0.180, 0.050)
//    , m_dialog_right(0.710, 0.880, 0.030, 0.050)
    , m_left(0.050, 0.100, 0.200, 0.700)
    , m_path_box(0.150, 0.020, 0.800, 0.780)
{}
void PathSelectDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_CYAN, m_bottom_right);
    items.add(COLOR_CYAN, m_dialog_left);
    items.add(COLOR_CYAN, m_dialog_middle);
//    items.add(COLOR_CYAN, m_dialog_right);
    items.add(COLOR_CYAN, m_left);
}
bool PathSelectDetector::detect(const ImageViewRGB32& screen) const{
    if (!PathScreenDetector::detect(screen)){
        return false;
    }

    ImageStats bottom_right = image_stats(extract_box_reference(screen, m_bottom_right));
    if (bottom_right.stddev.sum() < 30){
//        cout << "m_bottom_right = " << bottom_right.average << bottom_right.stddev << endl;
//        global_logger().log("m_bottom_right out");
        return false;
    }
    ImageStats dialog_left = image_stats(extract_box_reference(screen, m_dialog_left));
    if (!is_grey(dialog_left, 0, 200)){
//        global_logger().log("m_dialog_left out");
        return false;
    }
    ImageStats dialog_middle = image_stats(extract_box_reference(screen, m_dialog_middle));
    if (!is_grey(dialog_middle, 0, 300)){
//        global_logger().log("m_dialog_middle out");
        return false;
    }
//    ImageStats dialog_right = image_stats(extract_box_reference(screen, m_dialog_right));
//    if (!is_grey(dialog_right, 0, 200)){
//        return false;
//    }
    ImageStats left = image_stats(extract_box_reference(screen, m_left));
//    cout << left.average << left.stddev << endl;
    if (left.stddev.sum() < 100){
//        global_logger().log("m_left out");
        return false;
    }
//    if (euclidean_distance(dialog_left.average, dialog_right.average) > 10){
//        return false;
//    }
    if (dialog_left.average.sum() > dialog_middle.average.sum()){
//        global_logger_tagged().log("dialog out");
        return false;
    }
//    if (dialog_right.average.sum() > dialog_middle.average.sum()){
//        return false;
//    }

    ImageViewRGB32 path_box = extract_box_reference(screen, m_path_box);
    if (read_side(path_box) < 0){
//        cout << "PathSelectDetector(): read_side(screen) < 0" << endl;
        return false;
    }

//    cout << "PathSelectDetector(): Passed" << endl;
    return true;
}
bool PathSelectDetector::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    bool ret = detect(frame);
//    global_logger().log(std::to_string(ret));
    return ret;
}













PathReader::PathReader(VideoOverlay& overlay, size_t player_index)
    : m_path(overlay, {0.150, 0.020, 0.800, 0.780})
    , m_sprite0(overlay, {0.002, 0.345 + 0*0.16315, 0.071, 0.102})
    , m_sprite1(overlay, {0.002, 0.345 + 1*0.16315, 0.071, 0.102})
    , m_sprite2(overlay, {0.002, 0.345 + 2*0.16315, 0.071, 0.102})
    , m_sprite3(overlay, {0.002, 0.345 + 3*0.16315, 0.071, 0.102})
    , m_hp0(overlay, {0.074, 0.435 + 0*0.16315, player_index == 0 ? 0.052 : 0.041, 0.005})
    , m_hp1(overlay, {0.074, 0.435 + 1*0.16315, player_index == 1 ? 0.052 : 0.041, 0.005})
    , m_hp2(overlay, {0.074, 0.435 + 2*0.16315, player_index == 2 ? 0.052 : 0.041, 0.005})
    , m_hp3(overlay, {0.074, 0.435 + 3*0.16315, player_index == 3 ? 0.052 : 0.041, 0.005})
{}


void PathReader::read_sprites(Logger& logger, const ImageViewRGB32& screen, std::string slugs[4]) const{
    slugs[0] = read_pokemon_sprite_with_item(logger, screen, m_sprite0);
    slugs[1] = read_pokemon_sprite_with_item(logger, screen, m_sprite1);
    slugs[2] = read_pokemon_sprite_with_item(logger, screen, m_sprite2);
    slugs[3] = read_pokemon_sprite_with_item(logger, screen, m_sprite3);
    if (slugs[0].empty() || slugs[1].empty() || slugs[2].empty() || slugs[3].empty()){
        dump_image(logger, MODULE_NAME, "PathPartyReader-ReadSprites", screen);
    }
}
void PathReader::read_sprites(
    Logger& logger,
    GlobalState& state,
    const ImageViewRGB32& screen
) const{
    std::string mons[4];
    read_sprites(logger, screen, mons);
    state.add_seen(mons[0]);
    state.add_seen(mons[1]);
    state.add_seen(mons[2]);
    state.add_seen(mons[3]);
    if (!mons[0].empty()) state.players[0].pokemon = mons[0];
    if (!mons[1].empty()) state.players[1].pokemon = mons[1];
    if (!mons[2].empty()) state.players[2].pokemon = mons[2];
    if (!mons[3].empty()) state.players[3].pokemon = mons[3];
}


void PathReader::read_hp(Logger& logger, const ImageViewRGB32& screen, double hp[4]) const{
    hp[0] = read_hp_bar(logger, extract_box_reference(screen, m_hp0));
    hp[1] = read_hp_bar(logger, extract_box_reference(screen, m_hp1));
    hp[2] = read_hp_bar(logger, extract_box_reference(screen, m_hp2));
    hp[3] = read_hp_bar(logger, extract_box_reference(screen, m_hp3));
    if (hp[0] < 0 || hp[1] < 0 || hp[2] < 0 || hp[3] < 0){
        dump_image(logger, MODULE_NAME, "PathPartyReader-ReadHP", screen);
    }
}
void PathReader::read_hp(
    Logger& logger,
    GlobalState& state,
    const ImageViewRGB32& screen
) const{
    double hp[4];
    read_hp(logger, screen, hp);
    if (hp[0] >= 0) state.players[0].health.value.hp = hp[0];
    if (hp[1] >= 0) state.players[1].health.value.hp = hp[1];
    if (hp[2] >= 0) state.players[2].health.value.hp = hp[2];
    if (hp[3] >= 0) state.players[3].health.value.hp = hp[3];
    state.players[0].health.value.dead = 0;
    state.players[1].health.value.dead = 0;
    state.players[2].health.value.dead = 0;
    state.players[3].health.value.dead = 0;
}






void PathReader::read_path(
    ProgramEnvironment& env,
    VideoStream& stream, ProControllerContext& context,
    GlobalState& state
){
    PathMap path;
    if (MaxLairInternal::read_path(env, stream, context, path, m_path)){
        stream.log("Path Detection:\n" + path.dump(), COLOR_BLUE);
        state.path = path;
    }else{
        stream.log("Path Detection: Failed", COLOR_RED);
    }
}




void PathReader::read_side(
    Logger& logger,
    GlobalState& state,
    const ImageViewRGB32& screen
){
    int8_t path_side = MaxLairInternal::read_side(extract_box_reference(screen, m_path));
    switch (path_side){
    case 0:
        logger.log("Path Detection: Left side", COLOR_BLUE);
        break;
    case 1:
        logger.log("Path Detection: Right side", COLOR_BLUE);
        break;
    default:
        logger.log("Path Detection: Unable to read.", COLOR_RED);
        dump_image(logger, MODULE_NAME, "ReadPathSide", screen);
        break;
    }
    state.path_side = path_side;
}











}
}
}
}
