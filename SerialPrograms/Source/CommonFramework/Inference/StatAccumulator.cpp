/*  Stat Accumulator
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <algorithm>
#include <cmath>
#include <sstream>
#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/Logging/Logger.h"
#include "StatAccumulator.h"

namespace PokemonAutomation{



void StatAccumulatorI32::operator+=(uint32_t x){
    m_count++;
    m_sum += x;
    m_sqr += (uint64_t)x*x;
    m_min = std::min(m_min, x);
    m_max = std::max(m_max, x);
}

double StatAccumulatorI32::mean() const{
    return (double)m_sum / m_count;
}
double StatAccumulatorI32::stddev() const{
    double diff = m_sqr - (double)m_sum*m_sum / m_count;
    return std::sqrt(diff / (m_count - 1));
}


std::string StatAccumulatorI32::dump() const{
    std::stringstream ss;
    ss << "Count = " << tostr_u_commas(m_count);
    ss << ", Mean = " << mean();
    ss << ", Stddev = " << stddev();
    ss << ", Min = " << min();
    ss << ", Max = " << max();
    return ss.str();
}
void StatAccumulatorI32::log(Logger& logger, const std::string& label) const{
    logger.log(label + ": " + dump(), COLOR_MAGENTA);
}



FloatStatAccumulator::FloatStatAccumulator()
    : m_count(0)
    , m_sum(0)
    , m_sqr(0)
    , m_min(INFINITY)
    , m_max(-INFINITY)
{}

void FloatStatAccumulator::operator+=(double x){
    m_count++;
    m_sum += x;
    m_sqr += x*x;
    m_min = std::min(m_min, x);
    m_max = std::max(m_max, x);
}

double FloatStatAccumulator::mean() const{
    return m_sum / m_count;
}
double FloatStatAccumulator::stddev() const{
    return std::sqrt((m_sqr - m_sum*m_sum / m_count) / (m_count - 1));
}
double FloatStatAccumulator::diff_metric(double reference) const{
    return std::sqrt((m_sqr + reference*(reference * m_count - 2 * m_sum)) / m_count);
}




}
