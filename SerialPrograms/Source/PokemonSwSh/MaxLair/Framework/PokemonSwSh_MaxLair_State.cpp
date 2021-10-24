/*  Max Lair State
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <map>
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/Exception.h"
#include "PokemonSwSh_MaxLair_State.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


std::string PlayerState::dump() const{
    std::string str;
    str += console_id < 0 ? "NPC" : "Switch " + std::to_string(console_id);
    str += " - mon:";
    str += pokemon.empty() ? "?" : pokemon;
    str += " - item:";
    str += item.empty() ? "?" : item;
    str += " - dmax-turns-left:";
    str += std::to_string((int)dmax_turns_left);
    str += " - can-dmax:";
    str += can_dmax ? "yes" : "no";
    str += " - HP:";
    str += health.value.hp < 0 ? "?" : std::to_string(100*health.value.hp);
    str += "% - dead:";
    str += health.value.dead < 0 ? "?" : (health.value.dead ? "yes" : "no");
    str += " - PP:{";
    for (size_t c = 0; c < 4; c++){
        str += std::to_string(pp[c]);
        if (c + 1 < 4){
            str += ", ";
        }
    }
    str += "} - blocked:{";
    for (size_t c = 0; c < 4; c++){
        str += move_blocked[c] ? "1" : "0";
        if (c + 1 < 4){
            str += ", ";
        }
    }
    str += "}";
    return str;
}
void PlayerState::clear_battle_state(){
    dmax_turns_left = -1;
    for (size_t c = 0; c < 4 ; c++){
        move_blocked[c] = false;
    }
}

std::string PathMap::dump() const{
    std::string str;
    str += "    Path Type: " + std::to_string((int)path_type);
    str += "\n";
    {
        str += "        Row 3: ";
        bool first = true;
        for (size_t c = 0; c < 4; c++){
            if (!first){
                str += ", ";
            }
            first = false;
            str += get_type_slug(mon3[c]);
        }
        str += "\n";
    }
    {
        str += "        Row 2: ";
        bool first = true;
        for (size_t c = 0; c < 4; c++){
            if (!first){
                str += ", ";
            }
            first = false;
            str += get_type_slug(mon2[c]);
        }
        str += "\n";
    }
    {
        str += "        Row 1: ";
        bool first = true;
        for (size_t c = 0; c < 2; c++){
            if (!first){
                str += ", ";
            }
            first = false;
            str += get_type_slug(mon1[c]);
        }
        str += "\n";
    }
    return str;
}




std::string GlobalState::dump() const{
    std::string str;
    str += "    boss:";
    str += boss.empty() ? "?" : boss;
    str += " - wins:" + std::to_string(wins);
    str += " - lives:";
    str += lives_left < 0 ? "?" : std::to_string(lives_left);
    str += " - opponent:";
    if (opponent.empty()){
        str += "?";
    }else if (opponent.size() == 1){
        str += *opponent.begin();
    }else{
        str += set_to_str(opponent);
    }
    str += " - HP:";
    str += opponent_hp < 0 ? "?" : std::to_string(100 * opponent_hp);
    str += "% - slot:";
    str += std::to_string(move_slot);
    str += "\n";
    str += "    Player 0: " + players[0].dump() + "\n";
    str += "    Player 1: " + players[1].dump() + "\n";
    str += "    Player 2: " + players[2].dump() + "\n";
    str += "    Player 3: " + players[3].dump() + "\n";
    str += path.dump();
    switch (path_side){
    case 0: str += "        current-side: left"; break;
    case 1: str += "        current-side: right"; break;
    default: str += "        current-side: ?"; break;
    }
    str += "\n";
    return str;
}
void GlobalState::clear_battle_state(){
    opponent.clear();
    opponent_hp = -1;
    move_slot = 0;
    players[0].clear_battle_state();
    players[1].clear_battle_state();
    players[2].clear_battle_state();
    players[3].clear_battle_state();
}
size_t GlobalState::find_player_index(size_t console_id) const{
    for (size_t c = 0; c < 4; c++){
        if (players[c].console_id == (int8_t)console_id){
            return c;
        }
    }
    return console_id;
}










}
}
}
}
