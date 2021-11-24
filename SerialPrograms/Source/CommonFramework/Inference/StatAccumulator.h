/*  Stat Accumulator
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_StatAccumulator_H
#define PokemonAutomation_CommonFramework_StatAccumulator_H

#include <stdint.h>
#include <cmath>
#include <algorithm>
#include <string>
#include "Common/Cpp/PrettyPrint.h"

namespace PokemonAutomation{


class StatAccumulatorI32{
public:
    using StatObject = double;

public:
    void operator+=(uint32_t x){
        m_count++;
        m_sum += x;
        m_sqr += (uint64_t)x*x;
        m_min = std::min(m_min, x);
        m_max = std::max(m_max, x);
    }

    uint64_t count() const{ return m_count; };
    uint32_t min() const{ return m_min; }
    uint32_t max() const{ return m_max; }
    double mean() const{
        return (double)m_sum / m_count;
    }
    double stddev() const{
        double diff = m_sqr - (double)m_sum*m_sum / m_count;
        return std::sqrt(diff / (m_count - 1));
    }

    std::string dump() const{
        std::string str;
        str += "Count = " + tostr_u_commas(m_count);
        str += ", Mean = " + std::to_string(mean());
        str += ", Stddev = " + std::to_string(stddev());
        str += ", Min = " + std::to_string(min());
        str += ", Max = " + std::to_string(max());
        return str;
    }

private:
    uint64_t m_count = 0;
    uint64_t m_sum = 0;
    uint64_t m_sqr = 0;
    uint32_t m_min = (uint32_t)-1;
    uint32_t m_max = 0;
};



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


}
#endif
