/*  Blueberry Points Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_BlueberryPointsReader_H
#define PokemonAutomation_PokemonSV_BlueberryPointsReader_H

#include "Common/Cpp/Logging/AbstractLogger.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


// Read the numeric balance from a captured 16:9 quest panel; -1 on OCR failure.
int read_BP_from_image(Logger& logger, const ImageViewRGB32& screen);


}
}
}
#endif
