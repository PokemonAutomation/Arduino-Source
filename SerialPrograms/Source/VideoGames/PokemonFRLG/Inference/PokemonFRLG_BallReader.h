/*  Ball Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_BallReader_H
#define PokemonAutomation_PokemonFRLG_BallReader_H

#include <string>
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"

namespace PokemonAutomation{

class UnitTestDatabase;
class VideoStream;

namespace NintendoSwitch{
namespace PokemonFRLG{


class BallReader{
public:
    BallReader(VideoStream& stream);

    //  Read the ball slug from the first summary page. Return an empty string if no match is found.
    std::string read_ball(const ImageViewRGB32& screen) const;

private:
    VideoStream& m_stream;
    OverlayBoxScope m_box_sprite;
};

void add_tests_BallReader(UnitTestDatabase& database);


}
}
}
#endif
