/*  Raw Text Recognition
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_OCR_RawOCR_H
#define PokemonAutomation_OCR_RawOCR_H

#include <string>
#include "CommonFramework/Language.h"

namespace PokemonAutomation{
    class ImageViewRGB32;
namespace OCR{


bool language_available(Language language);

std::string ocr_read(Language language, const ImageViewRGB32& image);



}
}
#endif
