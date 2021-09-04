/*  IV Checker Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exception.h"
#include "Common/Qt/QtJsonTools.h"
#include "CommonFramework/Inference/ImageTools.h"
#include "CommonFramework/OCR/RawOCR.h"
#include "CommonFramework/OCR/Filtering.h"
#include "PokemonSwSh_IVCheckerReader.h"

#include <iostream>
using std::cout;
using std::endl;


namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


const std::map<std::string, IVCheckerReader::Result> IVCheckerReader::m_token_to_enum{
    {"No Good",         NoGood},
    {"Decent",          Decent},
    {"Pretty Good",     PrettyGood},
    {"Very Good",       VeryGood},
    {"Fantastic",       Fantastic},
    {"Best",            Best},
    {"Hyper trained!",  HyperTrained},
};
const std::map<IVCheckerReader::Result, std::string> IVCheckerReader::m_enum_to_token{
    {UnableToDetect,    "Unable to Detect"},
    {NoGood,            "No Good"},
    {Decent,            "Decent"},
    {PrettyGood,        "Pretty Good"},
    {VeryGood,          "Very Good"},
    {Fantastic,         "Fantastic"},
    {Best,              "Best"},
    {HyperTrained,      "Hyper trained!"},
};



IVCheckerReader::IVCheckerReader()
    : SmallDictionaryMatcher("PokemonSwSh/IVCheckerOCR.json")
{}

IVCheckerReader::Result IVCheckerReader::token_to_enum(const std::string& token){
    auto iter = m_token_to_enum.find(token);
    if (iter == m_token_to_enum.end()){
        return Result::UnableToDetect;
    }
    return iter->second;
}
const std::string& IVCheckerReader::enum_to_token(Result result){
    auto iter = m_enum_to_token.find(result);
    if (iter == m_enum_to_token.end()){
        PA_THROW_StringException("Invalid IV result enum.");
    }
    return iter->second;
}




IVCheckerReaderScope::IVCheckerReaderScope(const IVCheckerReader& reader, VideoOverlay& overlay, Language language)
    : m_reader(reader)
    , m_language(language)
    , m_box0(overlay, 0.777, 0.198 + 0 * 0.0515, 0.2, 0.0515)
    , m_box1(overlay, 0.777, 0.198 + 1 * 0.0515, 0.2, 0.0515)
    , m_box2(overlay, 0.777, 0.198 + 2 * 0.0515, 0.2, 0.0515)
    , m_box3(overlay, 0.777, 0.198 + 3 * 0.0515, 0.2, 0.0515)
    , m_box4(overlay, 0.777, 0.198 + 4 * 0.0515, 0.2, 0.0515)
    , m_box5(overlay, 0.777, 0.198 + 5 * 0.0515, 0.2, 0.0515)
{}


IVCheckerReader::Result IVCheckerReaderScope::read(Logger& logger, const QImage& frame, const InferenceBoxScope& box){
    QImage image = extract_box(frame, box);
    OCR::make_OCR_filter(image).apply(image);
//    image.save("test.png");

    QString text = OCR::ocr_read(m_language, image);

    OCR::MatchResult result = m_reader.match_substring(m_language, text);
    result.log(logger);
    if (!result.matched || result.slugs.size() != 1){
        return IVCheckerReader::Result::UnableToDetect;
    }
    return IVCheckerReader::token_to_enum(*result.slugs.begin());
}
IVCheckerReader::Results IVCheckerReaderScope::read(Logger& logger, const QImage& frame){
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

