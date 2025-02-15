/*  Nintendo Switch Navigation
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_Navigation_H
#define PokemonAutomation_NintendoSwitch_Navigation_H

namespace PokemonAutomation{
namespace NintendoSwitch{

class ProControllerContext;



void home_to_date_time(ProControllerContext& context, bool to_date_change, bool fast);



}
}
#endif
