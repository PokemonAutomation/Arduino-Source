/*  BDSP RNG State Reidentifier
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_StateReidentifier_H
#define PokemonAutomation_PokemonBDSP_StateReidentifier_H

#include <stddef.h>
#include <stdint.h>
#include <string>
#include <vector>
#include "Pokemon/Pokemon_Xorshift128.h"
#include "PokemonBDSP_BlinkModel.h"

namespace PokemonAutomation{
    class Cancellable;
    class Logger;
namespace NintendoSwitch{
namespace PokemonBDSP{


enum class ReidentifyMethod{
    Intervals,
    Types,
};


struct ReidentifyRequest{
    Pokemon::Xorshift128State base_state;
    uint64_t search_min = 0;
    uint64_t search_max = 1000000;
    uint8_t npcs = 1;
    ReidentifyMethod method = ReidentifyMethod::Intervals;
    std::vector<uint32_t> intervals;
    std::vector<BlinkType> types;
};

struct ReidentifyResult{
    bool success = false;
    bool ambiguous = false;
    size_t match_count = 0;
    uint64_t advances_to_first_blink = 0;
    uint64_t advances_to_last_blink = 0;
    std::string failure_reason;
};


ReidentifyResult reidentify_advances(
    const ReidentifyRequest& request,
    Logger* logger = nullptr
);


}
}
}
#endif
