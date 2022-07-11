/*  IV Checker Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QImage>
#include "PokemonBDSP_IVCheckerReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


IVCheckerReaderScope::IVCheckerReaderScope(VideoOverlay& overlay, Language language)
    : m_language(language)
    , m_box0(overlay, 0.785, 0.196 + 0 * 0.0529, 0.2, 0.0515)
    , m_box1(overlay, 0.785, 0.196 + 1 * 0.0529, 0.2, 0.0515)
    , m_box2(overlay, 0.785, 0.196 + 2 * 0.0529, 0.2, 0.0515)
    , m_box3(overlay, 0.785, 0.196 + 3 * 0.0529, 0.2, 0.0515)
    , m_box4(overlay, 0.785, 0.196 + 4 * 0.0529, 0.2, 0.0515)
    , m_box5(overlay, 0.785, 0.196 + 5 * 0.0529, 0.2, 0.0515)
{}


IVCheckerValue IVCheckerReaderScope::read(LoggerQt& logger, const QImage& frame, const InferenceBoxScope& box){
    ConstImageRef image = extract_box_reference(frame, box);
    OCR::StringMatchResult result = IVCheckerReader::instance().read_substring(
        logger, m_language, image,
        OCR::BLACK_TEXT_FILTERS()
    );
    result.clear_beyond_log10p(IVCheckerReader::MAX_LOG10P);
    if (result.results.size() != 1){
        return IVCheckerValue::UnableToDetect;
    }
    return IVCheckerValue_string_to_enum(result.results.begin()->second.token);
}
IVCheckerReader::Results IVCheckerReaderScope::read(LoggerQt& logger, const QImage& frame){
    IVCheckerReader::Results results;
    if (m_language != Language::None){
        results.hp      = read(logger, frame, m_box0);
        results.attack  = read(logger, frame, m_box1);
        results.defense = read(logger, frame, m_box2);
        results.spatk   = read(logger, frame, m_box3);
        results.spdef   = read(logger, frame, m_box4);
        results.speed   = read(logger, frame, m_box5);
    }
    return results;
}

std::vector<QImage> IVCheckerReaderScope::dump_images(const QImage& frame){
    std::vector<QImage> images;
    images.emplace_back(extract_box_copy(frame, m_box0));
    images.emplace_back(extract_box_copy(frame, m_box1));
    images.emplace_back(extract_box_copy(frame, m_box2));
    images.emplace_back(extract_box_copy(frame, m_box3));
    images.emplace_back(extract_box_copy(frame, m_box4));
    images.emplace_back(extract_box_copy(frame, m_box5));
    return images;
}



}
}
}

