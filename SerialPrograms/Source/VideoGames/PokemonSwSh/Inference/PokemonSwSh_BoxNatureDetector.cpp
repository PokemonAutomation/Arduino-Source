/*  IV Checker Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/ImageFilter.h"
#include "PokemonSwSh_BoxNatureDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

BoxNatureDetector::BoxNatureDetector(VideoOverlay& overlay)
    : m_box_atk(overlay, { 0.689, 0.198 + 1 * 0.0515, 0.084, 0.047})
    , m_box_def(overlay, { 0.689, 0.198 + 2 * 0.0515, 0.084, 0.047})
    , m_box_spatk(overlay, { 0.689, 0.198 + 3 * 0.0515, 0.084, 0.047})
    , m_box_spdef(overlay, { 0.689, 0.198 + 4 * 0.0515, 0.084, 0.047})
    , m_box_spd(overlay, { 0.689, 0.198 + 5 * 0.0515, 0.084, 0.047})
{}

NaturePlusMinus BoxNatureDetector::read(Logger& logger, const ImageViewRGB32& frame, const ImageFloatBox& box){
    const bool replace_color_within_range = true;

    //Filter out background - white/gray
    ImageRGB32 filtered_region = filter_rgb32_range(
        extract_box_reference(frame, box),
        combine_rgb(215, 215, 215), combine_rgb(255, 255, 255), Color(0), replace_color_within_range
    );
    ImageStats stats = image_stats(filtered_region);

    //filtered_region.save("./filtered_only.png");
    //cout << stats.average.r << endl;
    //cout << stats.average.g << endl;
    //cout << stats.average.b << endl;

    if (stats.average.b > stats.average.r + 50){
        return NaturePlusMinus::MINUS;
    }else if (stats.average.r > stats.average.b + 50){
        return NaturePlusMinus::PLUS;
    }
    return NaturePlusMinus::NEUTRAL;
}

NatureReader::Results BoxNatureDetector::read(Logger& logger, const ImageViewRGB32& frame){
    NatureReader::Results results;
    NaturePlusMinus stats[5];
    int statPlus = -1;
    int statMinus = -1;

    stats[0] = read(logger, frame, m_box_atk);
    stats[1] = read(logger, frame, m_box_def);
    stats[2] = read(logger, frame, m_box_spatk);
    stats[3] = read(logger, frame, m_box_spdef);
    stats[4] = read(logger, frame, m_box_spd);

    for (int i = 0; i < 5; i++){
        if (stats[i] == NaturePlusMinus::PLUS){
            statPlus = i;
        }else if (stats[i] == NaturePlusMinus::MINUS){
            statMinus = i;
        }
    }

    results.nature = NatureCheckerValue_helphinder_to_enum(std::make_pair(statPlus, statMinus));
    return results;
}


}
}
}

