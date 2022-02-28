/*  IV Checker Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Qt/QtJsonTools.h"
#include "CommonFramework/OCR/OCR_RawOCR.h"
#include "CommonFramework/OCR/OCR_Filtering.h"
#include "PokemonSwSh_IVCheckerReader.h"

#include <iostream>
using std::cout;
using std::endl;


namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{



IVCheckerReaderScope::IVCheckerReaderScope(VideoOverlay& overlay, Language language)
    : m_language(language)
    , m_box0(overlay, 0.777, 0.198 + 0 * 0.0515, 0.2, 0.0515)
    , m_box1(overlay, 0.777, 0.198 + 1 * 0.0515, 0.2, 0.0515)
    , m_box2(overlay, 0.777, 0.198 + 2 * 0.0515, 0.2, 0.0515)
    , m_box3(overlay, 0.777, 0.198 + 3 * 0.0515, 0.2, 0.0515)
    , m_box4(overlay, 0.777, 0.198 + 4 * 0.0515, 0.2, 0.0515)
    , m_box5(overlay, 0.777, 0.198 + 5 * 0.0515, 0.2, 0.0515)
{}


IVCheckerValue IVCheckerReaderScope::read(LoggerQt& logger, const QImage& frame, const InferenceBoxScope& box){
    QImage image = extract_box(frame, box);
    OCR::make_OCR_filter(image).apply(image);
//    image.save("test.png");

    QString text = OCR::ocr_read(m_language, image);

    static constexpr double MAX_LOG10P = -1.40;

    OCR::StringMatchResult result = IVCheckerReader::instance().match_substring(m_language, text);
    result.log(logger, MAX_LOG10P);
    result.clear_beyond_log10p(MAX_LOG10P);
    if (result.results.size() != 1){
        return IVCheckerValue::UnableToDetect;
    }
    return IVCheckerValue_string_to_enum(result.results.begin()->second.token);
}
IVCheckerReader::Results IVCheckerReaderScope::read(LoggerQt& logger, const QImage& frame){
    IVCheckerReader::Results results;
    results.hp      = read(logger, frame, m_box0);
    results.attack  = read(logger, frame, m_box1);
    results.defense = read(logger, frame, m_box2);
    results.spatk   = read(logger, frame, m_box3);
    results.spdef   = read(logger, frame, m_box4);
    results.speed   = read(logger, frame, m_box5);
    return results;
}

std::vector<QImage> IVCheckerReaderScope::dump_images(const QImage& frame){
    std::vector<QImage> images;
    images.emplace_back(extract_box(frame, m_box0));
    images.emplace_back(extract_box(frame, m_box1));
    images.emplace_back(extract_box(frame, m_box2));
    images.emplace_back(extract_box(frame, m_box3));
    images.emplace_back(extract_box(frame, m_box4));
    images.emplace_back(extract_box(frame, m_box5));
    return images;
}



}
}
}

