/*  Olive Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_OliveDetector_H
#define PokemonAutomation_PokemonSV_OliveDetector_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Notifications/ProgramInfo.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

class OliveDetector {
public:
    OliveDetector(VideoStream& stream, Color color = COLOR_RED);

    void make_overlays(VideoOverlaySet& items) const;

    std::pair<double, double> olive_location(VideoStream& stream, ProControllerContext& context, ImageFloatBox box = {0, 0.15, 1, 0.7});

    ImageFloatBox get_olive_floatbox(
        const ImageViewRGB32& screen, 
        ProControllerContext& context, 
        uint8_t rgb_gap,
        ImageFloatBox box
    );

    // return ImageFloatBox of the of the Olive, based on the largest blob of green
    ImageFloatBox get_olive_floatbox(
        VideoStream& stream,
        ProControllerContext& context, 
        uint8_t rgb_gap,
        ImageFloatBox box
    );

    ImageFloatBox align_to_olive(
        const ProgramInfo& info, 
        VideoStream& stream,
        ProControllerContext& context, 
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
        VideoStream& stream,
        ProControllerContext& context, 
        double direction_facing, 
        uint16_t total_forward_distance,
        uint16_t push_olive = 75,
        uint8_t rgb_gap = 20,
        ImageFloatBox area_to_check = {0, 0.3, 1.0, 0.40}  // {0, 0.15, 1, 0.7}
    );

    uint16_t walk_up_to_olive(
        const ProgramInfo& info, 
        VideoStream& stream,
        ProControllerContext& context, 
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
