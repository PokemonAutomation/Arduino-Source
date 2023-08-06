/*  Pokedex Menu Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageTools/BinaryImage_FilterRgb32.h"
#include "CommonFramework/ImageMatch/ExactImageMatcher.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "PokemonSV_PokedexMenuDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Kernels::Waterfill;


const ImageMatch::ExactImageMatcher& RECENTLY_BATTLED(){
    static ImageMatch::ExactImageMatcher matcher(RESOURCE_PATH() + "PokemonSV/LetsGoKill-Cropped.png");
    return matcher;
}

bool is_recently_battled_icon(
    const ImageViewRGB32& image,
    WaterfillObject& object,
    const WaterfillObject& red, const WaterfillObject& white
){
    if (red.max_y >= white.min_y){
        return false;
    }
    if (red.min_x < white.min_x){
        return false;
    }
    if (red.max_x < white.max_x){
        return false;
    }

    object = red;
    object.merge_assume_no_overlap(white);

    if (object.width() < 20 || object.height() < 20){
        return false;
    }
    double aspect_ratio = object.aspect_ratio();
    if (aspect_ratio < 0.8 || aspect_ratio > 1.2){
        return false;
    }

    ImageViewRGB32 cropped = extract_box_reference(image, object);
    double rmsd = RECENTLY_BATTLED().rmsd(cropped);
    return rmsd < 120;
}

RecentlyBattledMenuDetector::RecentlyBattledMenuDetector(Color color, const ImageFloatBox& box)
    : m_color(color)
    , m_box(box)
{}
void RecentlyBattledMenuDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}
bool RecentlyBattledMenuDetector::detect(const ImageViewRGB32& screen) const{
    using namespace Kernels::Waterfill;

    ImageViewRGB32 region = extract_box_reference(screen, m_box);

    std::vector<WaterfillObject> reds;
        std::vector<WaterfillObject> whites;
        std::unique_ptr<WaterfillSession> session = make_WaterfillSession();
        {
            std::vector<PackedBinaryMatrix> matrices = compress_rgb32_to_binary_range(
                region,
                {
                    {0xff802020, 0xffff7f7f},
                    {0xffc02020, 0xffff7f7f},
                }
            );

            for (PackedBinaryMatrix& matrix : matrices){
                session->set_source(matrix);
                auto iter = session->make_iterator(10);
                WaterfillObject object;
                while (iter->find_next(object, false)){
                    double aspect_ratio = object.aspect_ratio();
                    if (aspect_ratio < 1.5 || aspect_ratio > 2.5){
                        continue;
                    }
                    reds.emplace_back(std::move(object));
                }
            }
        }
        {
            std::vector<PackedBinaryMatrix> matrices = compress_rgb32_to_binary_range(
                region,
                {
                    {0xffc0c0c0, 0xffffffff},
                }
            );

            for (PackedBinaryMatrix& matrix : matrices){
                session->set_source(matrix);
                auto iter = session->make_iterator(10);
                WaterfillObject object;
                while (iter->find_next(object, false)){
                    double aspect_ratio = object.aspect_ratio();
                    if (aspect_ratio < 1.5 || aspect_ratio > 2.5){
                        continue;
                    }

                    whites.emplace_back(std::move(object));
                }
            }
        }
        for (WaterfillObject& red : reds){
            for (WaterfillObject& white : whites){
                WaterfillObject object;
                if (is_recently_battled_icon(region, object, red, white)){
                    return true;
                }
            }
        }

        return false;
}









}
}
}
