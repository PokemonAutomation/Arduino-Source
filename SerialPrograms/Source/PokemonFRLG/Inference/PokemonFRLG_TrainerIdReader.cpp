/*  Trainer ID Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PokemonFRLG_TrainerIdReader.h"
#include "Common/Cpp/Color.h"
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/Tools/GlobalThreadPools.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonTools/Images/ImageManip.h"
#include "CommonTools/OCR/OCR_NumberReader.h"
#include "CommonTools/OCR/OCR_Routines.h"
#include "PokemonFRLG/PokemonFRLG_Settings.h"
#include "PokemonFRLG_DigitReader.h"
#include <opencv2/imgproc.hpp>

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonFRLG {

TrainerIdReader::TrainerIdReader(Color color)
    : m_color(color)
    , m_box_tid(0.742683, 0.117314, 0.129734, 0.076006)
{}

void TrainerIdReader::make_overlays(VideoOverlaySet &items) const {
    const BoxOption &GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(m_color, GAME_BOX.inner_to_outer(m_box_tid));
}

uint16_t TrainerIdReader::read_tid(
    Logger &logger, const ImageViewRGB32 &frame
){
    ImageViewRGB32 game_screen =
            extract_box_reference(frame, GameSettings::instance().GAME_BOX);

    
    ImageViewRGB32 tid_region = extract_box_reference(game_screen, m_box_tid);

    if (!GlobalSettings::instance().USE_PADDLE_OCR){
        // Tesseract-free path: waterfill segmentation + template matching
        // against the PokemonFRLG/Digits/0-9.png templates.
        return uint16_t(read_digits_waterfill_template(logger, tid_region));
    }

    // PaddleOCR path (original): preprocess then per-digit waterfill OCR.
    ImageRGB32 ocr_ready = preprocess_for_ocr(
        tid_region, "TID", 7, 2, true,
        combine_rgb(0, 0, 0), combine_rgb(190, 190, 190)
    );

    // Waterfill isolates each digit -> per-char SINGLE_CHAR OCR.
    return uint16_t(OCR::read_number_waterfill(
        logger, ocr_ready, 0xff000000,
        0xff808080
    ));
}

} // namespace PokemonFRLG
} // namespace NintendoSwitch
} // namespace PokemonAutomation

