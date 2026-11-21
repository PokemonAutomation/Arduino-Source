/*  BDSP Battle Opponent Name Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "CommonTools/OCR/OCR_Routines.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonBDSP_BattleOpponentNameReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{

using namespace Pokemon;


// The doubles regions remain estimates; they are not used by the singles setup plans.
//
//  In BDSP singles, the one opponent's info box is in the top-right quadrant.
//  In BDSP doubles, two boxes sit side-by-side in the top half of the screen.
// Singles name strip excludes the HP bar and level on native game frames.
static const ImageFloatBox BOX_SINGLES_OPP   (0.740, 0.065, 0.160, 0.040);
static const ImageFloatBox BOX_DOUBLES_LEFT  (0.505, 0.010, 0.200, 0.085);
static const ImageFloatBox BOX_DOUBLES_RIGHT (0.710, 0.010, 0.260, 0.085);


static std::string ocr_name_box(
    Logger& logger,
    Language language,
    const ImageViewRGB32& frame,
    const ImageFloatBox& box
){
    ImageViewRGB32 cropped = extract_box_reference(frame, box);
    OCR::StringMatchResult result = PokemonNameReader::instance().read_substring(
        logger,
        language,
        cropped,
        OCR::BLACK_OR_WHITE_TEXT_FILTERS()
    );
    if (result.results.empty()){
        return "";
    }
    // Use the best match (lowest log10p) if it meets the confidence threshold
    double log10p = result.results.begin()->first;
    if (log10p > PokemonNameReader::MAX_LOG10P){
        return "";
    }
    return result.results.begin()->second.token;
}


std::vector<std::string> read_battle_opponent_names(
    Logger& logger,
    Language language,
    const ImageViewRGB32& frame,
    BDSPBattleType battle_type
){
    std::vector<std::string> names;
    if (battle_type == BDSPBattleType::SINGLES){
        names.push_back(ocr_name_box(logger, language, frame, BOX_SINGLES_OPP));
    } else {
        names.push_back(ocr_name_box(logger, language, frame, BOX_DOUBLES_LEFT));
        names.push_back(ocr_name_box(logger, language, frame, BOX_DOUBLES_RIGHT));
    }
    return names;
}


}
}
}
