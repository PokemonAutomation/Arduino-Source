/*  Max Lair Detect Path Map
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonSwSh/Inference/PokemonSwSh_TypeSymbolFinder.h"
//#include "PokemonSwSh/MaxLair/Options/PokemonSwSh_MaxLair_Options.h"
#include "PokemonSwSh_MaxLair_Detect_PathMap.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


bool read_type_array(
    VideoStream& stream,
    const ImageViewRGB32& screen,
    const ImageFloatBox& box,
    std::deque<OverlayBoxScope>& hits,
    size_t count,
    PokemonType* type, ImagePixelBox* boxes
){
    ImageViewRGB32 image = extract_box_reference(screen, box);
    for (size_t c = 0; c < count; c++){
        type[c] = PokemonType::NONE;
    }

    std::multimap<double, std::pair<PokemonType, ImagePixelBox>> candidates = find_type_symbols(screen, image, 0.20);
//    cout << candidates.size() << endl;

#if 0
    for (auto& item : candidates){
        cout << item.first << " : " << POKEMON_TYPE_SLUGS().get_string(item.second.first) << endl;
    }
#endif

    if (candidates.size() < count){
//        dump_image(console, screen, "ReadPath");
        return false;
    }

    std::multimap<size_t, const std::pair<PokemonType, ImagePixelBox>*> sorted;
//    std::deque<InferenceBoxScope> hits;
    hits.clear();
    size_t c = 0;
    for (const auto& item : candidates){
        hits.emplace_back(
            stream.overlay(),
            COLOR_GREEN,
            translate_to_parent(screen, box, item.second.second),
            POKEMON_TYPE_SLUGS().get_string(item.second.first)
        );
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

std::shared_ptr<const ImageRGB32> read_type_array_retry(
    VideoStream& stream, CancellableScope& scope,
    const ImageFloatBox& box,
    std::deque<OverlayBoxScope>& hits,
    size_t count,
    PokemonType* type, ImagePixelBox* boxes,
    size_t max_attempts = 3
){
    VideoSnapshot screen;
    for (size_t c = 0; c < max_attempts; c++){
        screen = stream.video().snapshot();
        if (read_type_array(stream, screen, box, hits, count, type, boxes)){
            return nullptr;
        }
        scope.wait_for(std::chrono::milliseconds(200));
    }
    return std::move(screen.frame);
}


bool read_path(
    ProgramEnvironment& env, VideoStream& stream, ProControllerContext& context,
    PathMap& path,
    const ImageFloatBox& box
){
    context.wait_for_all_requests();

    std::deque<OverlayBoxScope> hits;

    std::shared_ptr<const ImageRGB32> error_image;
    error_image = read_type_array_retry(stream, context, box, hits, 2, path.mon1, nullptr);
    if (error_image){
        dump_image(stream.logger(), env.program_info(), "ReadPath", *error_image);
        return false;
    }

    pbf_move_right_joystick(context, 128, 0, 70, 50);
    context.wait_for_all_requests();
    ImagePixelBox boxes[4];
    error_image = read_type_array_retry(stream, context, box, hits, 4, path.mon2, boxes);
    if (error_image){
        dump_image(stream.logger(), env.program_info(), "ReadPath", *error_image);
        return false;
    }

    while (true){
        pbf_move_right_joystick(context, 128, 0, 80, 50);
        context.wait_for_all_requests();

        error_image = read_type_array_retry(stream, context, box, hits, 4, path.mon3, nullptr);
        if (!error_image){
            break;
        }
        pbf_move_right_joystick(context, 128, 0, 20, 50);
        context.wait_for_all_requests();
        error_image = read_type_array_retry(stream, context, box, hits, 4, path.mon3, nullptr);
        if (!error_image){
            break;
        }

        dump_image(stream.logger(), env.program_info(), "ReadPath", *error_image);
        return false;
    }

    pbf_move_right_joystick(context, 128, 0, 125, 50);
    context.wait_for_all_requests();

    error_image = read_type_array_retry(stream, context, box, hits, 1, &path.boss, nullptr);
    if (error_image){
        dump_image(stream.logger(), env.program_info(), "ReadPath", *error_image);
        return false;
    }



    if (boxes[0].min_y < boxes[2].min_y &&
        boxes[2].min_y < boxes[1].min_y &&
        boxes[1].min_y < boxes[3].min_y
    ){
        path.path_type = 2;
        return true;
    }
    if (
        boxes[3].min_y > boxes[0].min_y &&
        boxes[0].min_y > boxes[1].min_y &&
        boxes[0].min_y > boxes[2].min_y
    ){
        path.path_type = 1;
        return true;
    }
    if (
        boxes[2].min_y < boxes[0].min_y &&
        boxes[0].min_y < boxes[1].min_y &&
        boxes[1].min_y < boxes[3].min_y
    ){
        path.path_type = 0;
        return true;
    }

    return false;
}



}
}
}
}
