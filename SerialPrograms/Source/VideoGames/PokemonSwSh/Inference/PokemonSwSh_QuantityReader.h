/*  Quantity Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_QuantityReader_H
#define PokemonAutomation_PokemonSwSh_QuantityReader_H

#include <stdint.h>
#include <string>
#include "CommonFramework/Logging/Logger.h"

namespace PokemonAutomation{
    class ImageViewRGB32;
namespace NintendoSwitch{
namespace PokemonSwSh{


struct ReadableQuantity999{
    bool read_error = false;
    bool unknown = true;
    uint16_t quantity = 0;

    std::string to_str() const;

    void update_with_ocr(
        int16_t new_quantity,
        int16_t max_quantity_differential = -1
    );
    void update_with_ocr(
        Logger& logger, const ImageViewRGB32& image,
        int16_t max_quantity_differential = -1
    );
};




}
}
}
#endif
