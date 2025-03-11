/*  Money Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_MoneyReader_H
#define PokemonAutomation_PokemonSV_MoneyReader_H

namespace PokemonAutomation{
    class Logger;
    class ImageViewRGB32;
namespace OCR{


//  Returns -1 if no number is found.
int read_money(Logger& logger, const ImageViewRGB32& image);


}
}
#endif
