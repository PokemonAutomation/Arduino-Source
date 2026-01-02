/*  Raw Text Recognition
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonTools_OCR_RawOCR_H
#define PokemonAutomation_CommonTools_OCR_RawOCR_H

#include <string>
#include "CommonFramework/Language.h"

namespace PokemonAutomation{
    class ImageViewRGB32;
namespace OCR{


// Check if Tesseract training data exists for the given language.
bool language_available(Language language);


//  OCR the image in the specified language.
//  Main OCR entry point. Performs OCR on the image using the specified language.
//  Thread-safe: internally uses a pool of Tesseract API instances, able to accept
//  multiple concurrent calls without delay or queueing.
//  It creates a new Tesseract instances if no available idle instance. You can
//  call `ensure_instances()` to pre-warm to pool with a given number of instances.
std::string ocr_read(Language language, const ImageViewRGB32& image);


//  Pre-warm the Tesseract API instance pool for a language by ensuring a minimum
//  number of instances exist.
//  Avoids lazy initialization delays during runtime. Thread-safe.
//  Call this if you expect to need to do many OCR instances in parallel and you
//  want to preload the OCR instances.
void ensure_instances(Language language, size_t instances);

//  Clear all TesseractAPI instances for all languages. Used for cleanup or
//  forcing re-initialization.
//  This is not safe to call while in any OCR is still running!
void clear_cache();



}
}
#endif
