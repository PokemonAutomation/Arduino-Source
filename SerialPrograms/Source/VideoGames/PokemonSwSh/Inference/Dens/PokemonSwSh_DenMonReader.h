/*  Den Mon Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_DenMonReader_H
#define PokemonAutomation_PokemonSwSh_DenMonReader_H

#include <string>
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Options/StringSelectOption.h"
#include "CommonTools/ImageMatch/ImageMatchResult.h"

namespace PokemonAutomation{
    namespace ImageMatch{
        class SilhouetteDictionaryMatcher;
    };
    class Logger;
    class VideoOverlay;
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
    DenMonReader(Logger& logger, VideoOverlay& overlay);

    DenMonReadResults read(const ImageViewRGB32& screen) const;

private:
    const ImageMatch::SilhouetteDictionaryMatcher& m_matcher;
    Logger& m_logger;
    OverlayBoxScope m_white;
    OverlayBoxScope m_den_color;
    OverlayBoxScope m_lair_pink;
    OverlayBoxScope m_sprite;
};




struct DenMonSelectData{
    DenMonSelectData();
    StringSelectDatabase m_database;
};

class DenMonSelectOption : private DenMonSelectData, public StringSelectOption{
public:
    DenMonSelectOption(std::string label);
};



}
}
}
#endif
