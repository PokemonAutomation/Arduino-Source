/*  Overworld Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "CommonTools/Images/BinaryImage_FilterRgb32.h"
#include "PokemonLA_OverworldDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{



OverworldDetector::OverworldDetector(Logger& logger, VideoOverlay& overlay)
    : VisualInferenceCallback("OverworldDetector")
    , m_arc_phone(logger, overlay, std::chrono::milliseconds(100), true)
{}

void OverworldDetector::make_overlays(VideoOverlaySet& items) const{
    m_arc_phone.make_overlays(items);
    m_mount.make_overlays(items);
}
bool OverworldDetector::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    m_arc_phone.process_frame(frame, timestamp);
    if (!m_arc_phone.detected()){
        return false;
    }
    return m_mount.detect(frame) != MountState::NOTHING;
}




bool is_pokemon_selection(VideoOverlay& overlay, const ImageViewRGB32& frame){
#if 1
    using namespace Kernels::Waterfill;

    OverlayBoxScope box(overlay, {0.83, 0.95, 0.11, 0.027});

    std::vector<PackedBinaryMatrix> matrices = compress_rgb32_to_binary_range(
        extract_box_reference(frame, box),
        {
            {0xff008000, 0xff40ffc0},
        }
    );
//    cout << matrices[0].dump() << endl;

    std::unique_ptr<WaterfillSession> session = make_WaterfillSession();
    for (PackedBinaryMatrix& matrix : matrices){
        session->set_source(matrix);
        auto iter = session->make_iterator(200);
        WaterfillObject object;
        while (iter->find_next(object, false)){
            //  Skip if it touches the boundary.
//            cout << object.min_x << " , " << object.min_y << endl;
            if (object.min_x == 0 || object.min_y == 0 ||
                object.min_x >= matrix.width() - 1 || object.min_y >= matrix.height() - 1
            ){
                continue;
            }

            //  Too short.
//            cout << object.width() << " x " << object.height() << endl;
            if (object.width() < object.height() * 10){
                continue;
            }

            return true;
        }
    }

    return false;
#else
    InferenceBoxScope box(overlay, 0.843, 0.96, 0.075, 0.005);
    ImageStats stats = image_stats(extract_box_reference(frame, box));
    cout << stats.average << stats.stddev << endl;
    extract_box_reference(frame, box).save("test.png");
    if (is_solid(stats, {0.0652401, 0.606812, 0.327948}, 0.15, 70)){
        return true;
    }
    return false;
#endif
}




}
}
}
