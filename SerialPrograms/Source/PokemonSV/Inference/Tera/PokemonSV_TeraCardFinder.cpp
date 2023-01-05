/*  Tera Card Finder
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "CommonFramework/ImageTools/BinaryImage_FilterRgb32.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/OCR/OCR_RawOCR.h"
#include "PokemonSV_TeraCardDetector.h"
#include "PokemonSV_TeraCodeReader.h"
#include "PokemonSV_TeraCardFinder.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


bool is_partial_tera_card(const ImageViewRGB32& image){
    if (!image){
        return false;
    }
    double aspect_ratio = (double)image.width() / image.height();
    if (aspect_ratio < 1.20 || aspect_ratio > 1.40){
        return false;
    }

    ImageStats white_top = image_stats(extract_box_reference(image, ImageFloatBox{0.60, 0.05, 0.20, 0.05}));
    if (!is_white(white_top)){
        return false;
    }
    ImageStats white_bottom = image_stats(extract_box_reference(image, ImageFloatBox{0.03, 0.95, 0.20, 0.03}));
    if (!is_white(white_bottom)){
        return false;
    }
    ImageStats names = image_stats(extract_box_reference(image, ImageFloatBox{0.05, 0.50, 0.20, 0.50}));
//        cout << names.average << names.stddev << endl;
    if (names.average.sum() < 500 || names.stddev.sum() < 100){
        return false;
    }
    ImageStats bottom_right = image_stats(extract_box_reference(image, ImageFloatBox{0.50, 0.50, 0.50, 0.50}));
//        cout << bottom_right.average << bottom_right.stddev << endl;
    if (euclidean_distance(white_top.average, bottom_right.average) < 50){
        return false;
    }

    return true;
}

std::string partial_tera_card_read_code(
    Logger& logger,
    const ImageViewRGB32& image
){
    ImageViewRGB32 box = extract_box_reference(image, ImageFloatBox{0.32, 0.08, 0.40, 0.11});
    return read_raid_code(logger, box);
}
std::map<Language, std::string> partial_tera_card_read_host(
    Logger& logger,
    const ImageViewRGB32& image,
    const std::set<Language>& languages
){
    std::map<Language, std::string> ret;

    ImageViewRGB32 box = extract_box_reference(image, ImageFloatBox(0.13, 0.60, 0.30, 0.07));
    std::string str = "Host: ";
    bool first = true;
    for (Language language : languages){
        ImageRGB32 filtered = filter_name_image(box);
        std::string raw = OCR::ocr_read(language, filtered);
        while (!raw.empty()){
            char ch = raw.back();
            if (ch <= 32 || ch == '/' || ch == '.'){
                raw.pop_back();
            }
            break;
        }
        if (!first){
            str += ", ";
        }
        first = false;
        str += language_data(language).code + "=\"" + raw + "\"";
        ret[language] = std::move(raw);
    }
    logger.log(str);
    return ret;
}





PartialTeraCardTracker::PartialTeraCardTracker(
    VideoOverlay& overlay, Color color, const ImageFloatBox& box,
    std::function<bool (const ImageViewRGB32& card)> on_card_callback
)
    : VisualInferenceCallback("PartialTeraCardTracker")
    , m_overlay(overlay)
    , m_color(color)
    , m_box(box)
    , m_on_card_callback(std::move(on_card_callback))
{}
void PartialTeraCardTracker::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}
bool PartialTeraCardTracker::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    using namespace Kernels::Waterfill;

    std::vector<PackedBinaryMatrix> matrices = compress_rgb32_to_binary_range(
        frame,
        {
//            {0xffc02010, 0xffff7f3f},   //  Scarlet
//            {0xff8040c0, 0xffdfbfff},   //  Violet
//            {0xffc08000, 0xffffff5f},   //  Event
//            {0xffc08000, 0xffffff5f},   //  6-Star
            {0xffc0c0a0, 0xffffffef},   //  Scarlet
            {0xffc0c0c0, 0xffffffff},   //  Everything else
        }
    );

    std::vector<ImagePixelBox> boxes;

//    size_t c = 0;
    std::unique_ptr<WaterfillSession> session = make_WaterfillSession();
    for (PackedBinaryMatrix& matrix : matrices){
        session->set_source(matrix);
        auto iter = session->make_iterator(10000);
        WaterfillObject object;
        while (iter->find_next(object, false)){
            ImageViewRGB32 image = extract_box_reference(frame, object);

            //  Not a Tera card. Move on.
            if (!is_partial_tera_card(image)){
                continue;
            }

            //  Check if it overlaps with an existing card. If so, skip.
//            image.save("test-" + std::to_string(c++) + ".png");
            bool overlaps = false;
            ImagePixelBox current_box = object;
            for (ImagePixelBox& box : boxes){
                if (box.overlaps_with(current_box)){
                    overlaps = true;
                    break;
                }
            }
            if (overlaps){
                continue;
            }

            if (m_on_card_callback){
                if (m_on_card_callback(image)){
                    return true;
                }
            }
            boxes.emplace_back(current_box);
        }
    }

//    cout << "boxes = " << boxes.size() << endl;

    SpinLockGuard lg(m_lock);
    m_overlays.clear();
    for (ImagePixelBox& box : boxes){
        m_overlays.emplace_back(m_overlay, COLOR_CYAN, translate_to_parent(frame, m_box, box), "");
    }

    return false;
}



}
}
}
