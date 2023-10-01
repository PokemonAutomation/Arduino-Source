/*  OCR Number Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_OCR_NumberReader_H
#define PokemonAutomation_OCR_NumberReader_H

#include <stdint.h>

namespace PokemonAutomation{
    class Logger;
    class ImageViewRGB32;
namespace OCR{


//  Returns -1 if no number is found.
//  No processing is done on the image. It is OCR'ed directly.
int read_number(Logger& logger, const ImageViewRGB32& image);


//  This version attempts to improve reliability by first isolating each number
//  via waterfill. Then it OCRs each number by itself and recombines them at the
//  end. This requires specifying the color range for the text.
int read_number_waterfill(
    Logger& logger, const ImageViewRGB32& image,
    uint32_t rgb32_min, uint32_t rgb32_max
 );



}
}
#endif
