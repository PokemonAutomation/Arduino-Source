/*  Outbreak Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTools/ImageStats.h"
//#include "CommonFramework/Tools/ErrorDumper.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonLA_OutbreakReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


OutbreakReader::OutbreakReader(Logger& logger, Language language, VideoOverlay& overlay)
    : m_logger(logger)
    , m_language(language)
    , m_dialog_box0(overlay, {0.030, 0.177, 0.020, 0.038})
    , m_dialog_box1(overlay, {0.030, 0.225, 0.020, 0.038})
    , m_text_box(overlay, {0.050, 0.177, 0.200, 0.038})
{}

void OutbreakReader::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_dialog_box0);
    items.add(COLOR_RED, m_dialog_box1);
}
OCR::StringMatchResult OutbreakReader::read(const ImageViewRGB32& screen) const{
    OCR::StringMatchResult result;
    ImageStats box0 = image_stats(extract_box_reference(screen, m_dialog_box0));
    ImageStats box1 = image_stats(extract_box_reference(screen, m_dialog_box1));
    double distance = euclidean_distance(box0.average, box1.average);
    if (distance < 20){
        m_logger.log("No outbreak found.", COLOR_ORANGE);
        return result;
    }

    ImageViewRGB32 image = extract_box_reference(screen, m_text_box);
    result = Pokemon::PokemonNameReader::instance().read_substring(
        m_logger, m_language, image,
        OCR::WHITE_TEXT_FILTERS()
    );
    result.clear_beyond_log10p(Pokemon::PokemonNameReader::MAX_LOG10P);
//    if (result.results.empty()){
//        dump_image(m_logger, ProgramInfo(), "OutbreakReader", screen);
//    }

    return result;
}


}
}
}
