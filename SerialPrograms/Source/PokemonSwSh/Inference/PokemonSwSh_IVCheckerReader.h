/*  IV Checker Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_IVCheckerReader_H
#define PokemonAutomation_PokemonSwSh_IVCheckerReader_H

#include "CommonFramework/Language.h"
#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/OCR/SmallDictionaryMatcher.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class IVCheckerReader : public OCR::SmallDictionaryMatcher{
public:
    enum Result{
        UnableToDetect,
        NoGood,
        Decent,
        PrettyGood,
        VeryGood,
        Fantastic,
        Best,
        HyperTrained,
    };

    struct Results{
        Result hp;
        Result attack;
        Result defense;
        Result spatk;
        Result spdef;
        Result speed;
    };

public:
    IVCheckerReader();

public:
    static Result token_to_enum(const std::string& token);
    static const std::string& enum_to_token(Result result);

private:
    static const std::map<std::string, Result> m_token_to_enum;
    static const std::map<Result, std::string> m_enum_to_token;
};


class IVCheckerReaderScope{
public:
    IVCheckerReaderScope(const IVCheckerReader& reader, VideoFeed& feed, Language language);

    IVCheckerReader::Results read(Logger* logger, const QImage& frame);

    std::vector<QImage> dump_images(const QImage& frame);

private:
    IVCheckerReader::Result read(Logger* logger, const QImage& frame, const InferenceBoxScope& box);

private:
    const IVCheckerReader& m_reader;
    Language m_language;
    InferenceBoxScope m_box0;
    InferenceBoxScope m_box1;
    InferenceBoxScope m_box2;
    InferenceBoxScope m_box3;
    InferenceBoxScope m_box4;
    InferenceBoxScope m_box5;
};




}
}
}
#endif
