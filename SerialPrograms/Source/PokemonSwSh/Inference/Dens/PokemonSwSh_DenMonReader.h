/*  Den Mon Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_DenMonReader_H
#define PokemonAutomation_PokemonSwSh_DenMonReader_H

#include <string>
#include "CommonFramework/Options/StringSelectOption.h"
#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/ImageMatch/CroppedImageMatcher.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


struct DenMonReadResults{
    //  TODO: Red vs. Purple
    //  TODO: Star count.
    //  TODO: Types
    ImageMatch::MatchResult slugs;
};

class DenMonReader{
public:
    DenMonReader(Logger& logger, VideoOverlay& overlay);

    DenMonReadResults read(const QImage& image, double max_RMSD_ratio = 0.20) const;

private:
    const ImageMatch::CroppedImageMatcher& m_matcher;
    Logger& m_logger;
    InferenceBoxScope m_box;
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
