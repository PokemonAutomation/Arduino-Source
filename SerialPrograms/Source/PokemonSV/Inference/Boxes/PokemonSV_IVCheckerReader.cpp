/*  IV Checker Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "PokemonSV_IVCheckerReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


const IVCheckerReader& IV_READER(){
    const static Pokemon::IVCheckerReader reader("PokemonSV/IVCheckerOCR.json");
    return reader;
}



IVCheckerReaderScope::IVCheckerReaderScope(VideoOverlay& overlay, Language language)
    : m_language(language)
    , m_box_hp(overlay, {0.825, 0.192, 0.110, 0.052})
    , m_box_attack(overlay, {0.886, 0.302, 0.110, 0.052})
    , m_box_defense(overlay, {0.886, 0.406, 0.110, 0.052})
    , m_box_spatk(overlay, {0.660, 0.302, 0.110, 0.052})
    , m_box_spdef(overlay, {0.660, 0.406, 0.110, 0.052})
    , m_box_speed(overlay, {0.825, 0.470, 0.110, 0.052})
{}


IVCheckerValue IVCheckerReaderScope::read(Logger& logger, const ImageViewRGB32& frame, const OverlayBoxScope& box){
    ImageViewRGB32 image = extract_box_reference(frame, box);
    OCR::StringMatchResult result = IV_READER().read_substring(
        logger, m_language, image,
        OCR::WHITE_TEXT_FILTERS()
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
        results.hp      = read(logger, frame, m_box_hp);
        results.attack  = read(logger, frame, m_box_attack);
        results.defense = read(logger, frame, m_box_defense);
        results.spatk   = read(logger, frame, m_box_spatk);
        results.spdef   = read(logger, frame, m_box_spdef);
        results.speed   = read(logger, frame, m_box_speed);
    }
    return results;
}

std::vector<ImageViewRGB32> IVCheckerReaderScope::dump_images(const ImageViewRGB32& frame){
    std::vector<ImageViewRGB32> images;
    images.emplace_back(extract_box_reference(frame, m_box_hp));
    images.emplace_back(extract_box_reference(frame, m_box_attack));
    images.emplace_back(extract_box_reference(frame, m_box_defense));
    images.emplace_back(extract_box_reference(frame, m_box_spatk));
    images.emplace_back(extract_box_reference(frame, m_box_spdef));
    images.emplace_back(extract_box_reference(frame, m_box_speed));
    return images;
}



}
}
}

