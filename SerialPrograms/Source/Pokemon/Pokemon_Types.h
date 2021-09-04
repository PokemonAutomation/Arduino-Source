/*  Pokemon Types
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Pokemon_Types_H
#define PokemonAutomation_Pokemon_Types_H

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


struct ShinyDetectionResult{
    ShinyType shiny_type;
    QImage best_screenshot;
};



}
}
#endif
