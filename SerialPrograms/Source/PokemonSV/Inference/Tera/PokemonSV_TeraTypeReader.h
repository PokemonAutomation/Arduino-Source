/*  Tera Type Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_TeraTypeReader_H
#define PokemonAutomation_PokemonSV_TeraTypeReader_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/ImageMatch/ImageMatchResult.h"
#include "CommonTools/ImageMatch/SilhouetteDictionaryMatcher.h"

namespace PokemonAutomation{
    struct ProgramInfo;
namespace NintendoSwitch{
namespace PokemonSV{



class TeraTypeReader{
public:
    TeraTypeReader(Color color = COLOR_BLUE);

    void make_overlays(VideoOverlaySet& items) const;
    ImageMatch::ImageMatchResult read(const ImageViewRGB32& screen) const;

private:
    const ImageMatch::SilhouetteDictionaryMatcher& m_matcher;
    Color m_color;
    ImageFloatBox m_box;
};



}
}
}
#endif
