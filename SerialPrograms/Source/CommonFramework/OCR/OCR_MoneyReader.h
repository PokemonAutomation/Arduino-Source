/*  OCR Money Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_OCR_MoneyReader_H
#define PokemonAutomation_OCR_MoneyReader_H

#include "CommonFramework/Logging/Logger.h"

namespace PokemonAutomation{
    class ImageViewRGB32;
namespace OCR{


//  Returns -1 if no number is found.
int read_money(Logger& logger, const ImageViewRGB32& image);


}
}
#endif
