/*  Threadpools for PaddleOCR
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonTools_OCR_RawPaddleOCR_H
#define PokemonAutomation_CommonTools_OCR_RawPaddleOCR_H

#include <string>
#include "CommonFramework/Language.h"

namespace PokemonAutomation{
    class ImageViewRGB32;
    namespace ML {
        class PaddleOCRPipeline;
    }
namespace OCR{


//  Pre-warm the PaddleOCR instance pool for a language. Ensure one instance exists.
//  Avoids lazy initialization delays during runtime. Thread-safe.
// returns a Paddle instance
ML::PaddleOCRPipeline& ensure_paddle_ocr_instance(Language language);

//  OCR the image in the specified language.
//  Main OCR entry point. Performs OCR on the image using the specified language.
//  Thread-safe: internally uses a pool of PaddleOCR instances, able to accept
//  multiple concurrent calls without delay or queueing.
//  It creates a new PaddleOCR instance if no available idle instance. You can
//  call `ensure_instances()` to pre-warm to pool with a given number of instances.
//
std::string paddle_ocr_read(
    Language language,
    const ImageViewRGB32& image
);



//  Clear all PaddleOCR instances for all languages. Used for cleanup or
//  forcing re-initialization.
//  This is not safe to call while any OCR is still running!
void clear_paddle_ocr_cache();



}
}
#endif
