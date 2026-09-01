/*  Origin Mark Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonHome_OriginMarkReader_H
#define PokemonAutomation_PokemonHome_OriginMarkReader_H

#include <optional>
#include <vector>
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/DetectedBoxes.h"
#include "Pokemon/Pokemon_OriginMarks.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{


using Pokemon::OriginMark;

class OriginMarkReader{
public:
    OriginMarkReader(Color color = COLOR_RED, VideoOverlay* overlay = nullptr);

    void make_overlays(VideoOverlaySet& items) const;

    OriginMark read_mark(
        const ImageViewRGB32& original_screen
    );

private:
    Color m_color;
    VideoOverlay* m_overlay;
    ImageFloatBox m_box;
    std::vector<DetectedBox> m_last_detected;
    std::optional<OverlayBoxScope> m_last_detected_box;

};

void add_tests_OriginMarkReader(UnitTestDatabase& database);

}
}
}
#endif