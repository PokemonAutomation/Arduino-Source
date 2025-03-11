/*  Read HP Bar
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Pokemon_ReadHpBar_H
#define PokemonAutomation_Pokemon_ReadHpBar_H

#include "CommonFramework/ImageTypes/ImageViewRGB32.h"

namespace PokemonAutomation{
    class Logger;
namespace Pokemon{



double read_hp_bar(const ImageViewRGB32& image);
double read_hp_bar(Logger& logger, const ImageViewRGB32& image);



}
}
#endif
