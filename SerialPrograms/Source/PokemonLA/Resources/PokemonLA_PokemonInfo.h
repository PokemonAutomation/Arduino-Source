/*  Pokemon LA Info
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_PokemonInfo_H
#define PokemonAutomation_PokemonLA_PokemonInfo_H

#include <QString>
namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

class PokemonDetails
{

public:
    QString name;
    QString gender;
    bool is_shiny = false;
    bool is_alpha = false;
};

}
}
}
#endif
