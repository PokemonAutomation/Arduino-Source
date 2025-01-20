/*  Ball Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonHome_BallReader_H
#define PokemonAutomation_PokemonHome_BallReader_H

#include <string>
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "CommonTools/ImageMatch/CroppedImageDictionaryMatcher.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{


class BallReader{
    static const double MAX_ALPHA;
    static const double ALPHA_SPREAD;

public:
    BallReader(VideoStream& stream);

public:
    std::string read_ball(const ImageViewRGB32& screen) const;

private:
    const ImageMatch::CroppedImageDictionaryMatcher& m_matcher;
    VideoStream& m_stream;
    OverlayBoxScope m_box_sprite;
};




}
}
}
#endif
