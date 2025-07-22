/*  Gradient Arrow Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <array>
#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTypes/BinaryImage.h"
#include "CommonTools/Images/BinaryImage_FilterRgb32.h"
#include "CommonTools/ImageMatch/ExactImageMatcher.h"
#include "PokemonSV_GradientArrowDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Kernels;
using namespace Kernels::Waterfill;



const ImageMatch::ExactImageMatcher& GRADIENT_ARROW_HORIZONTAL(){
    static ImageMatch::ExactImageMatcher matcher(RESOURCE_PATH() + "PokemonSV/GradientArrowHorizontal-Template.png");
    return matcher;
}
const ImageMatch::ExactImageMatcher& GRADIENT_ARROW_VERTICAL(){
    static ImageMatch::ExactImageMatcher matcher(RESOURCE_PATH() + "PokemonSV/GradientArrowVertical-Template.png");
    return matcher;
}

bool is_gradient_arrow(
    GradientArrowType type,
    const ImageViewRGB32& original_screen,
    const ImageViewRGB32& image,
    WaterfillObject& object,
    const WaterfillObject& yellow, const WaterfillObject& blue
){
    object = yellow;
    object.merge_assume_no_overlap(blue);

    size_t object_box_area = object.width() * object.height();
    double min_area = original_screen.total_pixels() * (2000. / (1920*1080));
    if (object_box_area < min_area){
        return false;
    }

    ImageViewRGB32 cropped = extract_box_reference(image, object);
//    cropped.save("test.png");

    const double THRESHOLD = 80;

    double aspect_ratio = object.aspect_ratio();
    switch (type){
    case GradientArrowType::RIGHT:{
        if (!(yellow.min_y < blue.min_y && blue.max_y < yellow.max_y)){
//            cout << "bad position" << endl;
            return false;
        }
        if (!(0.7 < aspect_ratio && aspect_ratio < 1.0)){
//            cout << "bad aspect ratio" << endl;
            return false;
        }
        double rmsd = GRADIENT_ARROW_HORIZONTAL().rmsd(cropped);
//        cout << "rmsd = " << rmsd << endl;
//        if (rmsd <= THRESHOLD){
//            cout << "object_box_area = " << object_box_area << endl;
//        }
        return rmsd <= THRESHOLD;
    }
    case GradientArrowType::DOWN:{
        if (!(yellow.min_x < blue.min_x && blue.max_x < yellow.max_x)){
//            cout << "bad position" << endl;
            return false;
        }
        if (!(1.0 < aspect_ratio && aspect_ratio < 1.43)){
//            cout << "bad aspect ratio" << endl;
            return false;
        }
        double rmsd = GRADIENT_ARROW_VERTICAL().rmsd(cropped);
//        cout << "rmsd = " << rmsd << endl;
        return rmsd <= THRESHOLD;
    }
    }

    return false;
}



GradientArrowDetector::GradientArrowDetector(
    Color color,
    GradientArrowType type,
    const ImageFloatBox& box
)
    : m_color(color)
    , m_type(type)
    , m_box(box)
{}
void GradientArrowDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}
bool GradientArrowDetector::detect(const ImageViewRGB32& screen){
    ImageFloatBox box;
    return detect(box, screen);
}

bool GradientArrowDetector::detect(ImageFloatBox& box, const ImageViewRGB32& screen) const{
    using namespace Kernels::Waterfill;

    ImageViewRGB32 region = extract_box_reference(screen, m_box);
//    region.save("test.png");

    std::vector<WaterfillObject> yellows;
    std::vector<WaterfillObject> blues;
    std::unique_ptr<WaterfillSession> session = make_WaterfillSession();
    {
        std::vector<PackedBinaryMatrix> matrices = compress_rgb32_to_binary_range(
            region,
            {
                {0xff808000, 0xffffff7f},
                {0xff808000, 0xffffff3f},

                {0xffa0a000, 0xffffff7f},
                {0xffa0a000, 0xffffff3f},

                {0xffc0c000, 0xffffff7f},
                {0xffc0c000, 0xffffff3f},

                {0xffe0e000, 0xffffff7f},
                {0xffe0e000, 0xffffff3f},

                {0xfff0f000, 0xffffff7f},
                {0xfff0f000, 0xffffff3f},

                {0xfff8f800, 0xffffff7f},
                {0xfff8f800, 0xffffff3f},
            }
        );

//        size_t c = 0;
//        PackedBinaryMatrix yellow_matrix = compress_rgb32_to_binary_range(region, 0xffc0c000, 0xffffff7f);
        for (PackedBinaryMatrix& matrix : matrices){
            session->set_source(matrix);
            auto iter = session->make_iterator(100);
            WaterfillObject object;
            while (iter->find_next(object, false)){
//                cout << "yellow = " << object.area << endl;
//                extract_box_reference(region, object).save("yellow-" + std::to_string(c++) + ".png");
                yellows.emplace_back(std::move(object));
            }
        }
    }
    {
        std::vector<std::pair<uint32_t, uint32_t>> filters;
        const std::array<uint8_t, 3> LG{0x40, 0x80, 0xc0};
        const std::array<uint8_t, 3> LB{0x80, 0xc0, 0xe0};
        const std::array<uint8_t, 4> HR{0x0f, 0x3f, 0x5f, 0x7f};
        const std::array<uint8_t, 2> HG{0xdf, 0xff};
        for (uint32_t lg : LG){
            for (uint32_t lb : LB){
                for (uint32_t hr : HR){
                    for (uint32_t hg : HG){
                        filters.emplace_back(
                            0xff000000 | (lg << 8) | (lb << 0),
                            0xff0000ff | (hr << 16) | (hg << 8)
                        );
                    }
                }
            }
        }
        std::vector<PackedBinaryMatrix> matrices = compress_rgb32_to_binary_range(
            region,
#if 1
            filters
#else
            {
                {0xff004080, 0xff7fffff},
                {0xff004080, 0xff5fffff},
                {0xff004080, 0xff3fffff},
                {0xff004080, 0xff0fffff},

                {0xff008080, 0xff7fffff},
                {0xff008080, 0xff5fffff},
                {0xff008080, 0xff3fffff},
                {0xff008080, 0xff0fffff},

                {0xff0080c0, 0xff7fffff},
                {0xff0080c0, 0xff5fffff},
                {0xff0080c0, 0xff3fffff},
                {0xff0080c0, 0xff0fffff},

                {0xff00c0c0, 0xff7fffff},
                {0xff00c0c0, 0xff5fffff},
                {0xff00c0c0, 0xff3fffff},
                {0xff00c0c0, 0xff0fffff},

                {0xff00c0e0, 0xff7fffff},
                {0xff00c0e0, 0xff5fffff},
                {0xff00c0e0, 0xff3fffff},
                {0xff00c0e0, 0xff0fffff},
            }
#endif
        );
//        PackedBinaryMatrix blue_matrix = compress_rgb32_to_binary_range(region, 0xff00c0c0, 0xff3fdfff);
//        cout << blue_matrix.dump() << endl;
//        size_t c = 0;
        for (PackedBinaryMatrix& matrix : matrices){
            session->set_source(matrix);
            auto iter = session->make_iterator(100);
            WaterfillObject object;
            while (iter->find_next(object, false)){
//                cout << "blue = " << object.area << endl;
//                extract_box_reference(region, object).save("blue-" + std::to_string(c++) + ".png");
                blues.emplace_back(std::move(object));
            }
        }
    }

//    std::vector<ImageFloatBox> hits;

//    size_t c = 0;
    for (WaterfillObject& yellow : yellows){
        for (WaterfillObject& blue : blues){
            WaterfillObject object;
            if (is_gradient_arrow(m_type, screen, region, object, yellow, blue)){
//                hits.emplace_back(translate_to_parent(screen, m_box, object));
//                extract_box_reference(region, object).save("object.png");
                box = translate_to_parent(screen, m_box, object);
                return true;
            }
//            double aspect_ratio = object.aspect_ratio();
//            cout << "aspect_ratio = " << aspect_ratio << endl;
//            extract_box_reference(region, object).save("object-" + std::to_string(c++) + ".png");

        }
    }

    return false;
}



#if 0
GradientArrowWatcher::~GradientArrowWatcher() = default;
GradientArrowWatcher::GradientArrowWatcher(
    Color color,
    GradientArrowType type,
    const ImageFloatBox& box
)
    : VisualInferenceCallback("GradientArrowWatcher")
    , m_detector(color, type, box)
{}

void GradientArrowWatcher::make_overlays(VideoOverlaySet& items) const{
    m_detector.make_overlays(items);
}
bool GradientArrowWatcher::process_frame(const VideoSnapshot& frame){
    bool detected = m_detector.detect(frame);
    if (detected){
//        m_last_detected = frame;
    }
    return detected;
}
//bool GradientArrowWatcher::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
//    return m_detector.detect(frame);
//}
#endif




}
}
}
