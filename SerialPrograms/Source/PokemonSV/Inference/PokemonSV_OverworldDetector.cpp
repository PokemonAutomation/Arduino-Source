/*  Overworld Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/ImageMatch/ExactImageMatcher.h"
#include "CommonFramework/ImageTools/BinaryImage_FilterRgb32.h"
#include "PokemonSV_OverworldDetector.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



const ImageMatch::ExactImageMatcher& RADAR_BALL(){
    static ImageMatch::ExactImageMatcher matcher(RESOURCE_PATH() + "PokemonSV/RadarBall.png");
    return matcher;
}




OverworldDetector::OverworldDetector(Color color)
    : m_color(color)
    , m_ball(0.890, 0.800, 0.030, 0.060)
    , m_radar(0.815, 0.680, 0.180, 0.310)
{}
void OverworldDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_ball);
    items.add(m_color, m_radar);
}
bool OverworldDetector::detect(const ImageViewRGB32& screen) const{
    if (!detect_ball(screen)){
        return false;
    }

    //  TODO: Detect the directions.

    return true;
}

bool OverworldDetector::detect_ball(const ImageViewRGB32& screen) const{
    using namespace Kernels::Waterfill;

    std::unique_ptr<WaterfillSession> session = make_WaterfillSession();
    ImageViewRGB32 image = extract_box_reference(screen, m_ball);
    std::vector<PackedBinaryMatrix> matrices = compress_rgb32_to_binary_range(
        image,
        {
            {0xffc0c000, 0xffffff1f},
            {0xffd0d000, 0xffffff1f},
            {0xffe0e000, 0xffffff1f},
            {0xfff0f000, 0xffffff1f},

            {0xffc0c000, 0xffffff3f},
            {0xffd0d000, 0xffffff3f},
            {0xffe0e000, 0xffffff3f},
            {0xfff0f000, 0xffffff3f},

            {0xffc0c000, 0xffffff5f},
            {0xffd0d000, 0xffffff5f},
            {0xffe0e000, 0xffffff5f},
            {0xfff0f000, 0xffffff5f},

            {0xffc0c000, 0xffffff7f},
            {0xffd0d000, 0xffffff7f},
            {0xffe0e000, 0xffffff7f},
            {0xfff0f000, 0xffffff7f},
        }
    );

//    size_t c = 0;
//    cout << "asdf" << endl;
    for (PackedBinaryMatrix& matrix : matrices){
        session->set_source(matrix);
        auto iter = session->make_iterator(50);
        WaterfillObject object;
        while (iter->find_next(object, false)){
            //  Exclude if it touches the borders.
            if (object.min_x == 0 || object.min_y == 0 ||
                object.max_x == image.width() || object.max_y == image.height()
            ){
                continue;
            }
            double aspect_ratio = object.aspect_ratio();
            if (!(0.8 < aspect_ratio && aspect_ratio < 1.2)){
                continue;
            }
            double area_ratio = object.area_ratio();
            if (!(0.5 < area_ratio && area_ratio < 0.9)){
                continue;
            }

//            extract_box_reference(image, object).save("ball-" + std::to_string(c++) + ".png");
            double rmsd = RADAR_BALL().rmsd(extract_box_reference(image, object));
//            cout << "rmsd = " << rmsd << endl;
            if (rmsd < 50){
                return true;
            }
        }
    }
    return false;
}




}
}
}
