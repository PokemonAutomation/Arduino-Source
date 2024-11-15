/*  Olive Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_OliveDetector_H
#define PokemonAutomation_PokemonSV_OliveDetector_H

#include "Common/Cpp/Color.h"
#include "ClientSource/Connection/BotBase.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

class OliveDetector {
public:
    OliveDetector(ConsoleHandle& console, Color color = COLOR_RED);

    void make_overlays(VideoOverlaySet& items) const;

    std::pair<double, double> olive_location(ConsoleHandle& console, BotBaseContext& context, ImageFloatBox box = {0, 0.15, 1, 0.7});

    ImageFloatBox get_olive_floatbox(
        const ImageViewRGB32& screen, 
        BotBaseContext& context, 
        uint8_t rgb_gap,
        ImageFloatBox box
    );

    // return ImageFloatBox of the of the Olive, based on the largest blob of green
    ImageFloatBox get_olive_floatbox(
        ConsoleHandle& console, 
        BotBaseContext& context, 
        uint8_t rgb_gap,
        ImageFloatBox box
    );

    ImageFloatBox align_to_olive(
        const ProgramInfo& info, 
        ConsoleHandle& console, 
        BotBaseContext& context, 
        double direction_facing, 
        uint8_t rgb_gap = 20,
        ImageFloatBox area_to_check = {0, 0.3, 1.0, 0.40}
    );

    // push the olive forward. 
    // move forward a certain number of ticks, as per total_forward_distance
    // always face a certain direction, as per direction_facing
    // return number of ticks walked
    uint16_t push_olive_forward(
        const ProgramInfo& info, 
        ConsoleHandle& console, 
        BotBaseContext& context, 
        double direction_facing, 
        uint16_t total_forward_distance,
        uint16_t push_olive = 75,
        uint8_t rgb_gap = 20,
        ImageFloatBox area_to_check = {0, 0.3, 1.0, 0.40}  // {0, 0.15, 1, 0.7}
    );

    uint16_t walk_up_to_olive(
        const ProgramInfo& info, 
        ConsoleHandle& console, 
        BotBaseContext& context, 
        double direction_facing, 
        uint8_t rgb_gap = 20,
        ImageFloatBox area_to_check = {0, 0.3, 1.0, 0.40}
    );
    

protected:
    VideoOverlaySet m_overlays;
    const Color m_color;
};



}
}
}
#endif
