/*  Ingo Opponent Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PokemonLA_IngoOpponent.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


const char* INGO_OPPONENT_STRINGS[] = {
    "Wenton",
    "Bren",
    "Zisu",
    "Akari/Rei",
    "Kamado",
    "Beni",
    "Ingo",
    "Ingo - but tougher",
    "Mai",
    "Sabi",
    "Ress",
    "Ingo - but using alphas",
};
const IngoOpponentMenuLocation INGO_OPPONENT_MENU_LOCATIONS_V10[] = {
    {0, 0},     //  Wenton
    {0, 1},     //  Bren
    {0, 2},     //  Zisu
    {0, 3},     //  Akari/Rei
    {1, 0},     //  Kamado
    {1, 1},     //  Beni
    {1, 2},     //  Ingo
    {-1, -1},   //  Ingo - but tougher
    {-1, -1},   //  Mai
    {-1, -1},   //  Sabi
    {-1, -1},   //  Ress
    {-1, -1},   //  Ingo - but using alphas
};
const IngoOpponentMenuLocation INGO_OPPONENT_MENU_LOCATIONS_V12[] = {
    {0, 0},     //  Wenton
    {0, 1},     //  Bren
    {0, 2},     //  Zisu
    {0, 3},     //  Akari/Rei
    {0, 4},     //  Kamado
    {1, 0},     //  Beni
    {1, 1},     //  Ingo
    {1, 2},     //  Ingo - but tougher
    {1, 3},     //  Mai
    {1, 4},     //  Sabi
    {2, 0},     //  Ress
    {2, 1},     //  Ingo - but using alphas
};


IngoOpponentOption::IngoOpponentOption()
    : EnumDropdownOption<IngoOpponents>(
        "<b>Opponent:</b>",
        {
            {IngoOpponents::Wenton,         "wenton",       "Wenton"},
            {IngoOpponents::Bren,           "bren",         "Bren"},
            {IngoOpponents::Zisu,           "zisu",         "Zisu"},
            {IngoOpponents::Akari_Rei,      "akari-rei",    "Akari/Rei"},
            {IngoOpponents::Kamado,         "kamado",       "Kamado"},
            {IngoOpponents::Beni,           "beni",         "Beni"},
            {IngoOpponents::Ingo,           "ingo",         "Ingo"},
            {IngoOpponents::Ingo_Tougher,   "ingo-tougher", "Ingo - but tougher"},
            {IngoOpponents::Mai,            "mai",          "Mai"},
            {IngoOpponents::Sabi,           "sabi",         "Sabi"},
            {IngoOpponents::Ress,           "ress",         "Ress"},
            {IngoOpponents::Ingo_Alphas,    "ingo-alphas",  "Ingo - but using alphas"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        IngoOpponents::Wenton
    )
{}





}
}
}
