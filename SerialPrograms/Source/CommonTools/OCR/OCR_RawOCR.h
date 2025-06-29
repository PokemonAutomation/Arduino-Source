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


bool language_available(Language language);


//  OCR the image in the specified language.
std::string ocr_read(Language language, const ImageViewRGB32& image);

//  Ensure that there are this many parallel instances for this language.
//  Call this if you expect to need to do many OCR instances in parallel and you
//  want to preload the OCR instances.
void ensure_instances(Language language, size_t instances);


}
}
#endif
