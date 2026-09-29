/*  Quantity Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonTools/OCR/OCR_NumberReader.h"
#include "PokemonSwSh_QuantityReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


std::string ReadableQuantity999::to_str() const{
    if (unknown){
        return "?";
    }
    std::string str = std::to_string(quantity);
    if (read_error){
        str += "?";
    }
    return str;
}

void ReadableQuantity999::update_with_ocr(
    int16_t new_quantity,
    int16_t max_quantity_differential
){
    //  Currently a read error. Keep last value.
    if (new_quantity < 0){
        read_error = true;
        return;
    }

    if (unknown){
        unknown = false;
        read_error = false;
        quantity = new_quantity;
        return;
    }

    unknown = false;
    if (max_quantity_differential < 0){
        read_error = false;
    }else{
        read_error = std::abs(quantity - new_quantity) > max_quantity_differential;
    }
    quantity = new_quantity;
}
void ReadableQuantity999::update_with_ocr(
    Logger& logger, const ImageViewRGB32& image,
    int16_t max_quantity_differential
){
    return update_with_ocr((int16_t)OCR::read_number(logger, image), max_quantity_differential);
}




}
}
}
