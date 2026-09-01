/*  BDSP RNG State Solver
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_StateSolver_H
#define PokemonAutomation_PokemonBDSP_StateSolver_H

#include <stddef.h>
#include <stdint.h>
#include <optional>
#include <string>
#include <vector>
#include "Pokemon/Pokemon_Xorshift128.h"
#include "PokemonBDSP_BlinkModel.h"

namespace PokemonAutomation{
    class Logger;
namespace NintendoSwitch{
namespace PokemonBDSP{


struct BlinkSolveResult{
    bool success = false;
    Pokemon::Xorshift128State state;
    uint64_t advances_to_last_blink = 0;
    size_t equations_used = 0;
    size_t null_space_dimension = 0;
    std::string failure_reason;
};


struct BlinkSample{
    uint64_t advance = 0;
    bool blinked = false;
    std::optional<BlinkType> type;
};


BlinkSolveResult solve_state_from_samples(
    const std::vector<BlinkSample>& samples, Logger* logger = nullptr
);


struct PokemonBlinkSolveRequest{
    std::vector<double> intervals;
    double tolerance_seconds = 0.1;
    size_t max_attempts = 64;
};


struct PokemonBlinkSolveResult{
    bool success = false;
    Pokemon::Xorshift128State state;
    uint64_t advances_to_last_interval = 0;
    size_t observations_used = 0;
    size_t observations_discarded = 0;
    size_t equations_used = 0;
    size_t null_space_dimension = 0;
    size_t attempts = 0;
    double weakest_margin_used = 0;
    size_t mistimed_intervals = 0;
    double worst_residual_seconds = 0;
    std::string failure_reason;
};


size_t recommended_pokemon_blink_count(double tolerance_seconds = 0.1);


PokemonBlinkSolveResult solve_state_from_pokemon_blinks(
    const PokemonBlinkSolveRequest& request,
    Logger* logger = nullptr
);


}
}
}
#endif
