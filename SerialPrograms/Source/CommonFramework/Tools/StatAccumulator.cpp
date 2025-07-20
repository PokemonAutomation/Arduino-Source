/*  Stat Accumulator
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include <cmath>
#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/Logging/Logger.h"
#include "StatAccumulator.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{



void StatAccumulatorI32::clear(){
    *this = StatAccumulatorI32();
}
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


std::string StatAccumulatorI32::dump(const char* units, double divider) const{
    divider = 1. / divider;
    std::string str;
    str += "Count = " + tostr_u_commas(m_count);
    str += ", Mean = " + tostr_default(mean() * divider) + units;
    str += ", Stddev = " + tostr_default(stddev() * divider) + units;
    str += ", Min = " + tostr_default(min() * divider) + units;
    str += ", Max = " + tostr_default(max() * divider) + units;
    return str;
}
void StatAccumulatorI32::log(Logger& logger, const std::string& label, const char* units, double divider) const{
    logger.log(label + ": " + dump(units, divider), COLOR_MAGENTA);
}



PeriodicStatsReporterI32::PeriodicStatsReporterI32(
    const char* label,
    const char* units, double divider,
    std::chrono::milliseconds period
)
    : m_label(label)
    , m_units(units)
    , m_divider(divider)
    , m_period(period)
    , m_last_report(current_time())
{}
void PeriodicStatsReporterI32::report_data(Logger& logger, uint32_t x){
    StatAccumulatorI32::operator+=(x);
    WallClock now = current_time();
    if (m_last_report + m_period <= now){
        log(logger, m_label, m_units, m_divider);
        clear();
        m_last_report = now;
    }

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
