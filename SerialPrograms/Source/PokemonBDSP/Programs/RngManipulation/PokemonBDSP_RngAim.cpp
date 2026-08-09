/*  BDSP RNG Aim
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cmath>
#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/Logging/AbstractLogger.h"
#include "PokemonBDSP_RngAim.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


uint64_t aimed_advance(uint64_t press, uint64_t span, int64_t bias){
    int64_t advance = (int64_t)(press + span) + bias;
    return advance < 0 ? 0 : (uint64_t)advance;
}

int64_t press_for_advance(uint64_t target, uint64_t span, int64_t bias){
    return (int64_t)target - (int64_t)span - bias;
}


RngAim::RngAim(size_t minimum_samples, double threshold)
    : m_minimum_samples(minimum_samples)
    , m_threshold(threshold)
{}

void RngAim::reset(){
    m_bias = 0;
    m_offsets.clear();
}

bool RngAim::record_offset(Logger& logger, int64_t offset, bool enabled){
    m_offsets.emplace_back(offset);
    if (!enabled || m_offsets.size() < m_minimum_samples){
        return false;
    }
    double mean = 0;
    for (int64_t o : m_offsets){
        mean += (double)o;
    }
    mean /= (double)m_offsets.size();
    if (std::abs(mean) < m_threshold){
        return false;
    }
    int64_t shift = (int64_t)std::llround(mean);
    if (shift == 0){
        return false;
    }
    m_bias += shift;
    logger.log(
        std::to_string(m_offsets.size()) + " measured attempt(s) average "
        + tostr_fixed(mean, 2) + " advance(s) off, so the aim shifts by "
        + std::to_string(shift) + " to " + std::to_string(m_bias) + ".",
        COLOR_BLUE
    );
    m_offsets.clear();
    return true;
}

std::string RngAim::describe_correction() const{
    if (m_bias == 0){
        return "";
    }
    return ", plus " + std::to_string(m_bias) + " measured";
}




}
}
}
