/*  Shiny Symbol Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Kernels/Waterfill/Kernels_Waterfill.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "CommonTools/Images/BinaryImage_FilterRgb32.h"
#include "CommonTools/ImageMatch/SubObjectTemplateMatcher.h"
#include "PokemonLA_ShinySymbolDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

using namespace Kernels;
using namespace Kernels::Waterfill;


const ImageFloatBox SHINY_SYMBOL_BOX_BOTTOM(0.32, 0.87, 0.03, 0.04);



class ShinySymbolDetector : public ImageMatch::SubObjectTemplateMatcher{
public:
    ShinySymbolDetector()
        : SubObjectTemplateMatcher("PokemonLA/ShinySymbol-Template1.png", COLOR_BLACK, 100)
    {
        PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(
            m_matcher.image_template(),
            128, 255,
            128, 255,
            128, 255
        );
        std::vector<WaterfillObject> objects = find_objects_inplace(matrix, 20);
        if (objects.size() != 2){
            throw FileException(
                nullptr, PA_CURRENT_FUNCTION,
                "Failed to find exactly 2 objects in resource.",
                m_path
            );
        }
        size_t index = 0;
        if (objects[0].area < objects[1].area){
            index = 1;
        }
        set_subobject(objects[index]);
//        cout << m_feature_box.x << ", "
//             << m_feature_box.y << ", "
//             << m_feature_box.width << ", "
//             << m_feature_box.height << endl;
    }

    static const ShinySymbolDetector& instance(){
        static ShinySymbolDetector matcher;
        return matcher;
    }
};



std::vector<ImagePixelBox> find_shiny_symbols(const ImageViewRGB32& image){
    PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(
        image,
        128, 255,
        128, 255,
        128, 255
    );
    std::vector<ImagePixelBox> ret;
    {
        PackedBinaryMatrix copy = matrix.copy();
        auto session = make_WaterfillSession(copy);
        auto finder = session->make_iterator(20);
        WaterfillObject object;
        while (finder->find_next(object, false)){
            ImagePixelBox object_box;
            if (ShinySymbolDetector::instance().matches_with_background_replace(object_box, image, matrix, object)){
                ret.emplace_back(object_box);
            }
        }
    }
#if 0
    cout << "objects = " << objects.size() << endl;
    static int c = 0;
    for (const auto& object : objects){
        image.copy(
            object.min_x, object.min_y, object.width(), object.height()
        ).save("test-" + std::to_string(c++) + ".png");
    }
#endif
    return ret;
}



ShinySymbolWatcher::ShinySymbolWatcher(VideoOverlay& overlay, const ImageFloatBox& box)
    : VisualInferenceCallback("ShinySymbolWatcher")
    , m_box(box)
    , m_overlays(overlay)
{}
void ShinySymbolWatcher::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_box);
}
bool ShinySymbolWatcher::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    m_matches = find_shiny_symbols(extract_box_reference(frame, m_box));
    m_overlays.clear();
    for (const ImagePixelBox& hit : m_matches){
        ImageFloatBox box = translate_to_parent(frame, m_box, hit);
        m_overlays.add(COLOR_CYAN, box);
    }
    return false;
}


bool ShinySymbolWaiter::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    ShinySymbolWatcher::process_frame(frame, timestamp);
    return !matches().empty();
}





}
}
}
