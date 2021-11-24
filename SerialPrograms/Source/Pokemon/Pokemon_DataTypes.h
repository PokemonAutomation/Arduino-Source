/*  Pokemon Types
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Pokemon_DataTypes_H
#define PokemonAutomation_Pokemon_DataTypes_H

#include <QImage>

namespace PokemonAutomation{
namespace Pokemon{


enum class ShinyType{
    UNKNOWN,
    NOT_SHINY,
    UNKNOWN_SHINY,
    STAR_SHINY,
    SQUARE_SHINY,
};
inline bool is_shiny(ShinyType type){
    switch (type){
    case ShinyType::UNKNOWN:
    case ShinyType::NOT_SHINY:
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
    QImage best_screenshot;
};



}
}
#endif
