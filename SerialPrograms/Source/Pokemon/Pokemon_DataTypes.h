/*  Pokemon Types
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Pokemon_DataTypes_H
#define PokemonAutomation_Pokemon_DataTypes_H

#include <memory>
#include "CommonFramework/ImageTypes/ImageRGB32.h"

namespace PokemonAutomation{
namespace Pokemon{


enum class ShinyType{
    UNKNOWN,
    NOT_SHINY,
    MAYBE_SHINY,    //  Unknown, but likely to be shiny.
    UNKNOWN_SHINY,
    STAR_SHINY,
    SQUARE_SHINY,
};
inline bool is_likely_shiny(ShinyType type){
    switch (type){
    case ShinyType::UNKNOWN:
    case ShinyType::NOT_SHINY:
        return false;
    case ShinyType::MAYBE_SHINY:
    case ShinyType::UNKNOWN_SHINY:
    case ShinyType::STAR_SHINY:
    case ShinyType::SQUARE_SHINY:
        return true;
    }
    return false;
}
inline bool is_confirmed_shiny(ShinyType type){
    switch (type){
    case ShinyType::UNKNOWN:
    case ShinyType::NOT_SHINY:
    case ShinyType::MAYBE_SHINY:
        return false;
    case ShinyType::UNKNOWN_SHINY:
    case ShinyType::STAR_SHINY:
    case ShinyType::SQUARE_SHINY:
        return true;
    }
    return false;
}


struct ShinyDetectionResult{
    ShinyType shiny_type = ShinyType::UNKNOWN;
    double alpha = 0;
    std::shared_ptr<const ImageRGB32> best_screenshot;

    ImageViewRGB32 get_best_screenshot() const{
        return best_screenshot ? (ImageViewRGB32)*best_screenshot : ImageViewRGB32();
    }
};



}
}
#endif
