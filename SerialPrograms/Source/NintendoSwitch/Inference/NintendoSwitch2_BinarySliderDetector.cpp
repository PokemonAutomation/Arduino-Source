/*  Binary Slider Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/BinaryImage_FilterRgb32.h"
#include "CommonTools/ImageMatch/ExactImageMatcher.h"
#include "NintendoSwitch2_BinarySliderDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{



BinarySliderDetector::BinarySliderDetector(Color color, const ImageFloatBox& box)
    : m_color(color)
    , m_box(box)
{}
void BinarySliderDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}

std::vector<std::pair<bool, ImagePixelBox>> BinarySliderDetector::detect(const ImageViewRGB32& screen) const{
    using namespace Kernels::Waterfill;

    static ImageMatch::ExactImageMatcher LIGHT_OFF_CURSOR   (RESOURCE_PATH() + "NintendoSwitch2/BinarySlider/BinarySlider-Light-Off-Cursor.png");
    static ImageMatch::ExactImageMatcher LIGHT_OFF_NOCURSOR (RESOURCE_PATH() + "NintendoSwitch2/BinarySlider/BinarySlider-Light-Off-NoCursor.png");
    static ImageMatch::ExactImageMatcher LIGHT_ON_CURSOR    (RESOURCE_PATH() + "NintendoSwitch2/BinarySlider/BinarySlider-Light-On-Cursor.png");
    static ImageMatch::ExactImageMatcher LIGHT_ON_NOCURSOR  (RESOURCE_PATH() + "NintendoSwitch2/BinarySlider/BinarySlider-Light-On-NoCursor.png");
    static ImageMatch::ExactImageMatcher DARK_OFF_CURSOR    (RESOURCE_PATH() + "NintendoSwitch2/BinarySlider/BinarySlider-Dark-Off-Cursor.png");
    static ImageMatch::ExactImageMatcher DARK_OFF_NOCURSOR  (RESOURCE_PATH() + "NintendoSwitch2/BinarySlider/BinarySlider-Dark-Off-NoCursor.png");
    static ImageMatch::ExactImageMatcher DARK_ON_CURSOR     (RESOURCE_PATH() + "NintendoSwitch2/BinarySlider/BinarySlider-Dark-On-Cursor.png");
    static ImageMatch::ExactImageMatcher DARK_ON_NOCURSOR   (RESOURCE_PATH() + "NintendoSwitch2/BinarySlider/BinarySlider-Dark-On-NoCursor.png");

    ImageViewRGB32 region = extract_box_reference(screen, m_box);

    std::vector<PackedBinaryMatrix> matrices = compress_rgb32_to_binary_range(
        region,
        {
            {0xffc0c0c0, 0xffffffff},
            {0xffd0d0d0, 0xffffffff},
            {0xffe0e0e0, 0xffffffff},
            {0xfff0f0f0, 0xffffffff},
        }
    );

    auto session = make_WaterfillSession();

    std::vector<std::pair<bool, ImagePixelBox>> best;

//    int c = 0;
    for (PackedBinaryMatrix& matrix : matrices){
//        cout << "-----------------" << endl;
        session->set_source(matrix);
        auto iter = session->make_iterator(200);
        WaterfillObject object;

        std::vector<std::pair<bool, ImagePixelBox>> sliders;

        while (iter->find_next(object, false)){
            double aspect_ratio = object.aspect_ratio();
            if (aspect_ratio < 0.9 || aspect_ratio > 1.1){
                continue;
            }

            double min_area = screen.total_pixels() * (3000. / (3840*2160));
            if ((double)object.area < min_area){
                continue;
            }

            ImageViewRGB32 cropped = extract_box_reference(region, object);

            double best_off_rmsd = 9999;
            best_off_rmsd = std::min(best_off_rmsd, LIGHT_OFF_CURSOR.rmsd(cropped));
            best_off_rmsd = std::min(best_off_rmsd, LIGHT_OFF_NOCURSOR.rmsd(cropped));
            best_off_rmsd = std::min(best_off_rmsd, DARK_OFF_CURSOR.rmsd(cropped));
            best_off_rmsd = std::min(best_off_rmsd, DARK_OFF_NOCURSOR.rmsd(cropped));

            double best_on_rmsd = 9999;
            best_on_rmsd = std::min(best_on_rmsd, LIGHT_ON_CURSOR.rmsd(cropped));
            best_on_rmsd = std::min(best_on_rmsd, LIGHT_ON_NOCURSOR.rmsd(cropped));
            best_on_rmsd = std::min(best_on_rmsd, DARK_ON_CURSOR.rmsd(cropped));
            best_on_rmsd = std::min(best_on_rmsd, DARK_ON_NOCURSOR.rmsd(cropped));

    //        cout << "best_off_rmsd = " << best_off_rmsd << endl;
    //        cout << "best_on_rmsd  = " << best_on_rmsd << endl;

            bool on = best_on_rmsd < best_off_rmsd;
            double best_rmsd = on ? best_on_rmsd : best_off_rmsd;

            if (best_rmsd < 60){
//                cout << "rmsd = " << best_rmsd << endl;
//                cropped.save("slider-" + std::to_string(c) + "-" + std::to_string(sliders.size()) + ".png");
                sliders.emplace_back(on, object);
            }

//            for (auto& item : sliders){
//                cout << item.first << " : " << item.second.max_y << endl;
//            }

        }

        if (best.size() < sliders.size()){
            best = std::move(sliders);
        }

//        c++;
    }

    return best;
}



}
}
