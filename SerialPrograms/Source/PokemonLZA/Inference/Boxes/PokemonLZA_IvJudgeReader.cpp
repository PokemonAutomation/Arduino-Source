/*  IV Judge Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "PokemonLZA_IvJudgeReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{


const IvJudgeReader& IV_READER(){
    const static Pokemon::IvJudgeReader reader("PokemonLZA/IVCheckerOCR.json");
    return reader;
}




IvJudgeReaderScope::IvJudgeReaderScope(VideoOverlay& overlay, Language language)
    : m_language(language)
    , m_box0(overlay, {0.765000, 0.255000, 0.085000, 0.045000})
    , m_box1(overlay, {0.850000, 0.335000, 0.085000, 0.045000})
    , m_box2(overlay, {0.855000, 0.435000, 0.085000, 0.045000})
    , m_box3(overlay, {0.675000, 0.335000, 0.085000, 0.045000})
    , m_box4(overlay, {0.675000, 0.435000, 0.085000, 0.045000})
    , m_box5(overlay, {0.765000, 0.510000, 0.085000, 0.045000})
{}




IvJudgeValue IvJudgeReaderScope::read(Logger& logger, const ImageViewRGB32& frame, const OverlayBoxScope& box){
    ImageViewRGB32 image = extract_box_reference(frame, box);
    OCR::StringMatchResult result = IV_READER().read_substring(
        logger, m_language, image,
        OCR::LIME_TEXT_FILTERS()
        );
    result.clear_beyond_log10p(IvJudgeReader::MAX_LOG10P);
    if (result.results.size() != 1){
        return IvJudgeValue::UnableToDetect;
    }
    return IV_JUDGE_VALUE_STRINGS().get_enum(result.results.begin()->second.token);
}
IvJudgeReader::Results IvJudgeReaderScope::read(Logger& logger, const ImageViewRGB32& frame){
    IvJudgeReader::Results results;
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

std::vector<ImageViewRGB32> IvJudgeReaderScope::dump_images(const ImageViewRGB32& frame){
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

