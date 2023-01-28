/*  IV Checker Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Color.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageFilter.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "PokemonSwSh_BoxNatureDetector.h"

#include <iostream>
using std::cout;
using std::endl;

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
    const ImageStats region = image_stats(extract_box_reference(frame, box));

    //cout << region.average.r << endl;
    //cout << region.average.g << endl;
    //cout << region.average.b << endl;
    
    if (region.average.b > region.average.r + 3) {
        return NaturePlusMinus::MINUS;
    }
    else if (region.average.r > region.average.b) {
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

    for (int i = 0; i < 5; i++) {
        if (stats[i] == NaturePlusMinus::PLUS) {
            statPlus = i;
        }
        else if (stats[i] == NaturePlusMinus::MINUS) {
            statMinus = i;
        }
    }

    results.nature = NatureCheckerValue_helphinder_to_enum(std::make_pair(statPlus, statMinus));
    return results;
}


}
}
}

