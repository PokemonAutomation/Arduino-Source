/*  IV Checker Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "PokemonSwSh_IVCheckerReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{



IVCheckerReaderScope::IVCheckerReaderScope(VideoOverlay& overlay, Language language)
    : m_language(language)
    , m_box0(overlay, {0.777, 0.198 + 0 * 0.0515, 0.2, 0.0515})
    , m_box1(overlay, {0.777, 0.198 + 1 * 0.0515, 0.2, 0.0515})
    , m_box2(overlay, {0.777, 0.198 + 2 * 0.0515, 0.2, 0.0515})
    , m_box3(overlay, {0.777, 0.198 + 3 * 0.0515, 0.2, 0.0515})
    , m_box4(overlay, {0.777, 0.198 + 4 * 0.0515, 0.2, 0.0515})
    , m_box5(overlay, {0.777, 0.198 + 5 * 0.0515, 0.2, 0.0515})
{}


IVCheckerValue IVCheckerReaderScope::read(Logger& logger, const ImageViewRGB32& frame, const OverlayBoxScope& box){
    ImageViewRGB32 image = extract_box_reference(frame, box);
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
IVCheckerReader::Results IVCheckerReaderScope::read(Logger& logger, const ImageViewRGB32& frame){
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

std::vector<ImageViewRGB32> IVCheckerReaderScope::dump_images(const ImageViewRGB32& frame){
    std::vector<ImageViewRGB32> images;
    images.emplace_back(extract_box_reference(frame, m_box0));
    images.emplace_back(extract_box_reference(frame, m_box1));
    images.emplace_back(extract_box_reference(frame, m_box2));
    images.emplace_back(extract_box_reference(frame, m_box3));
    images.emplace_back(extract_box_reference(frame, m_box4));
    images.emplace_back(extract_box_reference(frame, m_box5));
    return images;
}



}
}
}

