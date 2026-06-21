/*  BDSP AutoStory - Game Data
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Provides access to move metadata, pokemon base stats, and the trainer
 *  database used by the outside-help battle handler.
 */

#ifndef PokemonAutomation_PokemonBDSP_AutoStory_BDSPData_H
#define PokemonAutomation_PokemonBDSP_AutoStory_BDSPData_H

#include <string>
#include <vector>
#include "Pokemon/Pokemon_Types.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{

using namespace Pokemon;


// ---------------------------------------------------------------------------
//  Move data
// ---------------------------------------------------------------------------

struct BDSPMoveData{
    MoveCategory category;
    PokemonType  type;
    uint8_t      base_power;
    uint8_t      pp;
    bool         spread;
    uint8_t      accuracy;  // 0 = never-miss (e.g. Swift); otherwise 1-100
};

//  Returns move data for the given slug (e.g. "earthquake").
//  Throws InternalProgramError if the slug is not found.
const BDSPMoveData& bdsp_move(const std::string& slug);


// ---------------------------------------------------------------------------
//  Pokemon base stats
// ---------------------------------------------------------------------------

struct BDSPPokemonData{
    PokemonType type[2];   // type[1] == NONE for mono-type pokemon
    uint8_t     hp;
    uint8_t     attack;
    uint8_t     defense;
    uint8_t     spatk;
    uint8_t     spdef;
    uint8_t     speed;
};

//  Returns base stats for the given species slug (e.g. "beautifly").
//  Throws InternalProgramError if the slug is not found.
const BDSPPokemonData& bdsp_pokemon(const std::string& slug);


// ---------------------------------------------------------------------------
//  Trainer database
// ---------------------------------------------------------------------------

enum class BDSPBattleType{
    SINGLES,
    DOUBLES,
};

struct BDSPStatSet{
    uint8_t hp      = 0;
    uint8_t attack  = 0;
    uint8_t defense = 0;
    uint8_t spatk   = 0;
    uint8_t spdef   = 0;
    uint8_t speed   = 0;
};

struct BDSPTrainerPokemon{
    std::string species;
    uint8_t     level;
    BDSPStatSet ivs;
    BDSPStatSet evs;
    bool        has_intimidate = false;
};

struct BDSPTrainer{
    std::vector<BDSPTrainerPokemon> pokemon;
};

struct BDSPTrainerData{
    BDSPBattleType         battle_type;
    std::vector<BDSPTrainer> trainers;
};

//  Returns trainer data for the given battle ID (e.g. "Veilstone_GalacticGrunts_2v2").
//  Throws InternalProgramError if the ID is not found.
const BDSPTrainerData& bdsp_trainer(const std::string& battle_id);


}
}
}
#endif
