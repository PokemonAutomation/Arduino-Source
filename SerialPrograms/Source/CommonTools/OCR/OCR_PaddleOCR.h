/*  Threadpools for PaddleOCR
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

// NOTE: This file should only be inluded in OCR_Routines.h
// This file should not be inluded in other files.
// Use OCR_Routines.h instead

#ifndef PokemonAutomation_CommonTools_OCR_PaddleOCR_H
#define PokemonAutomation_CommonTools_OCR_PaddleOCR_H

#include <string>
#include "CommonFramework/Language.h"
#include "OCR_RawTesseractOCR.h"

namespace PokemonAutomation{
    class ImageViewRGB32;
    namespace ML {
        class PaddleOCRPipeline;
    }
namespace OCR{

bool paddle_ocr_language_available(Language language);


//  Pre-warm the PaddleOCR instance pool for a language. Ensure one instance exists.
//  Avoids lazy initialization delays during runtime. Thread-safe.
//  returns a pointer to a Paddle instance, for the given language.
ML::PaddleOCRPipeline& ensure_paddle_ocr_instance(Language language);

//  OCR the image in the specified language.
//  Main OCR entry point. Performs OCR on the image using the specified language.
//  Thread-safe: internally uses a pool of PaddleOCR instances, able to accept
//  multiple concurrent calls without delay or queueing.
//  It creates one PaddleOCR instance for each language. You can
//  call `ensure_instances()` to pre-warm to pool with a given number of instances.
//
//  psm: Page segmentation mode, same meaning as in `tesseract_ocr_read()`.
//       Defaults to SINGLE_LINE.
//     - AUTO, SINGLE_BLOCK, SINGLE_COLUMN: multi-line OCR. Runs the PaddleOCR text
//       detection model to find every line of text in the image, then recognizes each
//       line. Lines are returned top to bottom, separated by '\n'. Use this when the
//       image is not pre-cropped to a single line, e.g. a whole dialog box.
//     - Any other mode: single-line OCR. Assumes the image is already cropped to one
//       line of text and skips the detection model.
std::string paddle_ocr_read(
    Language language,
    const ImageViewRGB32& image,
    PageSegMode psm = PageSegMode::SINGLE_LINE
);



//  Clear all PaddleOCR instances for all languages. Used for cleanup or
//  forcing re-initialization.
//  This is not safe to call while any OCR is still running!
void clear_paddle_ocr_cache();



}
}
#endif
