/*  BDSP RNG Aim
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_RngAim_H
#define PokemonAutomation_PokemonBDSP_RngAim_H

#include <stddef.h>
#include <stdint.h>
#include <string>
#include <vector>

namespace PokemonAutomation{
    class Logger;
namespace NintendoSwitch{
namespace PokemonBDSP{


//  The advance where a press will generate the target Pokemon
uint64_t aimed_advance(uint64_t press, uint64_t span, int64_t bias);

//  The press that would reach "target" through a schedule of a certain length
int64_t press_for_advance(uint64_t target, uint64_t span, int64_t bias);


class RngAim{
public:
    explicit RngAim(size_t minimum_samples = 3, double threshold = 1.0);

    int64_t bias() const{ return m_bias; }

    void reset();

    bool record_offset(Logger& logger, int64_t offset, bool enabled);

    std::string describe_correction() const;

private:
    size_t m_minimum_samples;
    double m_threshold;

    int64_t m_bias = 0;
    std::vector<int64_t> m_offsets;
};


}
}
}
#endif
