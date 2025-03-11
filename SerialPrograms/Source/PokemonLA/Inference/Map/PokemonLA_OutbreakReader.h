/*  Outbreak Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *  
 *  Read Outbreak pokemon text on map when leaving village
 */

#ifndef PokemonAutomation_PokemonLA_OutbreakReader_H
#define PokemonAutomation_PokemonLA_OutbreakReader_H

#include "CommonFramework/Language.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/OCR/OCR_StringMatchResult.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class OutbreakReader{
public:
    OutbreakReader(Logger& logger, Language language, VideoOverlay& overlay);

    void make_overlays(VideoOverlaySet& items) const;
    OCR::StringMatchResult read(const ImageViewRGB32& screen) const;


private:
    Logger& m_logger;
    Language m_language;
    OverlayBoxScope m_dialog_box0;
    OverlayBoxScope m_dialog_box1;
    OverlayBoxScope m_text_box;
};



}
}
}
#endif
