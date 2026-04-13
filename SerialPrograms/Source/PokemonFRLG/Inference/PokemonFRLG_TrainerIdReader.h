/*  Trainer ID Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_TrainerIdReader_H
#define PokemonAutomation_PokemonFRLG_TrainerIdReader_H

#include <optional>
#include <string>
#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Language.h"


namespace PokemonAutomation{

class Logger;
class ImageViewRGB32;
class VideoOverlaySet;

namespace NintendoSwitch{
namespace PokemonFRLG{

class TrainerIdReader {
public:
    TrainerIdReader(Color color = COLOR_RED);

    void make_overlays(VideoOverlaySet &items) const;

    // Reads the Trainer ID on the Trainer Card
    uint16_t read_tid(
        Logger &logger, const ImageViewRGB32 &frame
    );

private:
    Color m_color;
    ImageFloatBox m_box_tid;

};

} // namespace PokemonFRLG
} // namespace NintendoSwitch
} // namespace PokemonAutomation
#endif

