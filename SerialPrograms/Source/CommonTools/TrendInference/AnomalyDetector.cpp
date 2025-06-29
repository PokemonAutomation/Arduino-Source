/*  Differential Anomaly Detector_H
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cmath>
#include <algorithm>
#include "AnomalyDetector.h"

namespace PokemonAutomation{


AnomalyDetector::AnomalyDetector(size_t window_size, double min_value, double max_value)
    : m_window_size(window_size)
    , m_min_value(min_value)
    , m_max_value(max_value)
    , m_sum(0)
    , m_sum_sqr(0)
{
    double range = max_value - min_value;
    double max_size = range * range * window_size;
    m_scale_to_fixedpoint = std::sqrt(((uint64_t)1 << 63) / max_size);
    m_scale_to_double = 1. / m_scale_to_fixedpoint;
}
double AnomalyDetector::push(double x){
    if (m_window.size() >= m_window_size){
        uint32_t fixed = m_window.front();
        m_window.pop_front();
        m_sum -= fixed;
        m_sum_sqr -= (uint64_t)fixed * fixed;
    }

    x -= m_min_value;
    x = std::max(x, 0.);
    x = std::min(x, m_max_value - m_min_value);

    uint32_t fixed = (uint32_t)(x * m_scale_to_fixedpoint);
    m_window.emplace_back(fixed);
    m_sum += fixed;
    m_sum_sqr += (uint64_t)fixed * fixed;

    return sigma(x);
}
double AnomalyDetector::mean() const{
    size_t count = m_window.size();
    return m_sum / (double)count * m_scale_to_double + m_min_value;
}
double AnomalyDetector::stddev() const{
    size_t count = m_window.size();
    return std::sqrt((m_sum_sqr - m_sum*m_sum / (double)count) / (count - 1)) * m_scale_to_double;
}
double AnomalyDetector::sigma(double x) const{
    return std::abs((x - mean()) / stddev());
}



TimeNormalizedDeltaAnomalyDetector::TimeNormalizedDeltaAnomalyDetector(size_t window_size, double max_value)
    : m_window_size(window_size)
    , m_max_value(max_value)
    , m_sum(0)
    , m_sum_sqr(0)
{
    double max_size = max_value * max_value * window_size;
    m_scale_to_fixedpoint = std::sqrt(((uint64_t)1 << 63) / max_size);
    m_scale_to_double = 1. / m_scale_to_fixedpoint;
}
double TimeNormalizedDeltaAnomalyDetector::push(
    double x,
    std::chrono::time_point<std::chrono::system_clock> timestamp
){
    if (m_window.size() >= m_window_size){
        uint32_t fixed = m_window.front().fixed_point_value;
        m_window.pop_front();
        m_sum -= fixed;
        m_sum_sqr -= (uint64_t)fixed * fixed;
    }

    if (m_window.empty()){
        m_window.emplace_back();
        Frame& frame = m_window.back();
        frame.timestamp = timestamp;
        frame.fixed_point_value = 0;
        return 0;
    }

    auto time_diff = timestamp - m_window.back().timestamp;
    uint32_t millis = (uint32_t)std::chrono::duration_cast<std::chrono::milliseconds>(time_diff).count();
    x /= millis;
    x = std::min(x, m_max_value);

    uint32_t fixed = (uint32_t)(x * m_scale_to_fixedpoint);

    m_window.emplace_back();
    Frame& frame = m_window.back();
    frame.timestamp = timestamp;
    frame.fixed_point_value = fixed;
    m_sum += fixed;
    m_sum_sqr += (uint64_t)fixed * fixed;


//        cout << x << " / " << stddev() << " = " << sigma(x) << endl;
//        cout << "count     = " << m_window.size() << endl;
//        cout << "sum       = " << m_sum << endl;
//        cout << "m_sum_sqr = " << m_sum_sqr << endl;
    return sigma(x);
}

double TimeNormalizedDeltaAnomalyDetector::mean() const{
    size_t count = m_window.size();
    return m_sum / (double)count * m_scale_to_double;
}
double TimeNormalizedDeltaAnomalyDetector::stddev() const{
    size_t count = m_window.size();
    return std::sqrt((m_sum_sqr - m_sum*m_sum / (double)count) / (count - 1)) * m_scale_to_double;
}
double TimeNormalizedDeltaAnomalyDetector::sigma(double x) const{
    return std::abs((x - mean()) / stddev());
}



}

