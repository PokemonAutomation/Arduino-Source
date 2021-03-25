/*  Float Stat Accumulator
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_FloatStatAccumulator_H
#define PokemonAutomation_CommonFramework_FloatStatAccumulator_H

#include <stdint.h>
#include <cmath>
#include <algorithm>

class FloatStatAccumulator{
public:
    using StatObject = double;

public:
    void operator+=(double x){
        m_count++;
        m_sum += x;
        m_sqr += x*x;
        m_min = std::min(m_min, x);
        m_max = std::max(m_max, x);
    }

    uint64_t count() const{ return m_count; };
    double min() const{ return m_min; }
    double max() const{ return m_max; }
    double mean() const{
        return m_sum / m_count;
    }
    double stddev() const{
        return std::sqrt((m_sqr - m_sum*m_sum / m_count) / (m_count - 1));
    }
    double diff_metric(double reference) const{
        return std::sqrt((m_sqr + reference*(reference * m_count - 2 * m_sum)) / m_count);
    }

private:
    uint64_t m_count = 0;
    double m_sum = 0;
    double m_sqr = 0;
    double m_min = INFINITY;
    double m_max = -INFINITY;
};


#endif
