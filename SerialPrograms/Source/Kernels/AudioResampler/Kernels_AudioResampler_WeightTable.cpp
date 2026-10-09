/*  Audio Resampler - Weight Table
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <numeric>
#include <cmath>
#include "Common/Cpp/Containers/AlignedVector.tpp"
#include "Kernels/Kernels_Alignment.h"
#include "Kernels/KaiserWindow/Kernels_KaiserWindow.h"
#include "Kernels_AudioResampler_WeightTable.h"

//  REMOVE
#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace Kernels{
namespace AudioResampler{



WeightTable::WeightTable(
    size_t in_sample_rate,
    size_t out_sample_rate,
    uint32_t taps,
    double beta,
    size_t min_samples
)
    : m_taps(taps)
{
    using namespace Kernels::KaiserWindow;

    double ratio = (double)in_sample_rate / out_sample_rate;
    double ibeta = bessel_i0(beta);
    double tap_radius = taps * 0.5;
    double downsample_scale = 1.0;
    if (in_sample_rate > out_sample_rate){
        downsample_scale = (double)out_sample_rate / in_sample_rate;
    }
//    uint32_t window_shift = taps / 2;
//    cout << "window_shift = " << window_shift << endl;

    size_t gcd = std::gcd(in_sample_rate, out_sample_rate);
//    size_t in_period = in_sample_rate / gcd;
//    size_t out_period = out_sample_rate / gcd;
    size_t period = out_sample_rate / gcd;
    m_period = period;

    size_t width = std::max(period, min_samples);

    //  Calculate the starting input indices.
    {
        m_index = AlignedVector<uint32_t>(width);
        for (size_t c = 0; c < width; c++){
            m_index[c] = (uint32_t)std::round(c * ratio);
//            cout << "target = " << target << endl;
//            cout << center << ",";
        }
//        cout << endl;
    }

    m_tap_stride = width * sizeof(float);

    //  Round up to the cache way.
    constexpr size_t CACHE_WAY = 4096;
    m_tap_stride = Kernels::align_int_up<CACHE_WAY>(m_tap_stride);

    //  Extra padding to break superalignment.
    m_tap_stride += 4096 / taps;

    //  Round up to min alignment.
    m_tap_stride = Kernels::align_int_up<PA_ALIGNMENT>(m_tap_stride);
    m_tap_stride /= sizeof(float);


    m_data = AlignedVector<float>(taps * m_tap_stride);
    memset(m_data.data(), 0, taps * m_tap_stride * sizeof(float));
//    cout << "items = " << taps * m_tap_stride << endl;

    for (uint32_t tap_index = 0; tap_index < taps; tap_index++){
        float* tap_row = internal_tap_start(tap_index);
        double tap_shift = tap_index - tap_radius + 0.5;
//        cout << "-----------------" << endl;
//        cout << "tap_shift = " << tap_shift << endl;

        for (size_t c = 0; c < width; c++){
            double in_index = m_index[c] + tap_shift;
            double x = in_index - c * ratio;
//            cout << "in[" << in_index << "]: x = " << x << endl;
            if (x == 0){
                tap_row[c] = (float)downsample_scale;
                continue;
            }
            if (std::abs(x) >= tap_radius){
                tap_row[c] = 0;
                continue;
            }

            x *= downsample_scale;

            double arg = 3.1415926535897932385 * x;
            double sinc = std::sin(arg) / arg;
//            cout << "sinc = " << sinc << endl;

            x /= tap_radius;
            x *= x;
            x = 1 - x;
            x = x <= 0 ? 0 : std::sqrt(x);
            x *= beta;
            x = bessel_i0(x) / ibeta;
//            cout << "bessel = " << x << endl;
            x *= sinc;
            x *= downsample_scale;

            //  Truncate small values that are likely zero.
            if (std::abs(x) < 1e-12){
                x = 0;
            }

//            cout << x << endl;
            tap_row[c] = (float)x;
        }
    }
}






}
}
}
