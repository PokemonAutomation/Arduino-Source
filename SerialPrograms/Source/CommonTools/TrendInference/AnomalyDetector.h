/*  Differential Anomaly Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonTools_DifferentialAnomalyDetector_H
#define PokemonAutomation_CommonTools_DifferentialAnomalyDetector_H

#include <deque>
#include "Common/Cpp/Time.h"

namespace PokemonAutomation{


class AnomalyDetector{
public:
    AnomalyDetector(size_t window_size, double min_value, double max_value);
    double push(double x);

    double mean() const;
    double stddev() const;
    double sigma(double x) const;

private:
    size_t m_window_size;
    double m_min_value;
    double m_max_value;
    double m_scale_to_fixedpoint;
    double m_scale_to_double;
    uint64_t m_sum;
    uint64_t m_sum_sqr;
    std::deque<uint32_t> m_window;
};


class TimeNormalizedDeltaAnomalyDetector{
public:
    TimeNormalizedDeltaAnomalyDetector(size_t window_size, double max_value);
    double push(double x, WallClock timestamp = current_time());

    double mean() const;
    double stddev() const;
    double sigma(double x) const;

private:
    struct Frame{
        WallClock timestamp;
        uint32_t fixed_point_value;
    };

    size_t m_window_size;
    double m_max_value;
    double m_scale_to_fixedpoint;
    double m_scale_to_double;
    uint64_t m_sum;
    uint64_t m_sum_sqr;
    std::deque<Frame> m_window;
};





}

#endif
