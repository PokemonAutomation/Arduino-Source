/*  Den Mon Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_DenMonReader_H
#define PokemonAutomation_PokemonSwSh_DenMonReader_H

#include <string>
#include "CommonFramework/Options/StringSelectOption.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonFramework/ImageMatch/SilhouetteDictionaryMatcher.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


struct DenMonReadResults{
    enum DenType{
        NOT_DETECTED,
        RED_BEAM,
        PURPLE_BEAM,
        MAX_LAIR,
    };

    //  TODO: Star count.
    //  TODO: Types
    DenType type = NOT_DETECTED;
    ImageMatch::ImageMatchResult slugs;

};

class DenMonReader{
    static constexpr double MAX_ALPHA = 100;
    static constexpr double ALPHA_SPREAD = 20;

public:
    DenMonReader(LoggerQt& logger, VideoOverlay& overlay);

    DenMonReadResults read(const QImage& screen) const;

private:
    const ImageMatch::SilhouetteDictionaryMatcher& m_matcher;
    LoggerQt& m_logger;
    InferenceBoxScope m_white;
    InferenceBoxScope m_den_color;
    InferenceBoxScope m_lair_pink;
    InferenceBoxScope m_sprite;
};




struct DenMonSelectData{
    DenMonSelectData();
    const std::vector<std::pair<QString, QIcon>>& cases() const{ return m_list; }

protected:
    std::vector<std::pair<QString, QIcon>> m_list;
};

class DenMonSelectOption : private DenMonSelectData, public StringSelectOption{
public:
    DenMonSelectOption(QString label);

    std::string slug() const;

private:
};



}
}
}
#endif
