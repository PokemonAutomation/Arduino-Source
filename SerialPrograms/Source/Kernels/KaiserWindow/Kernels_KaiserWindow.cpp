/*  Kaiser Window
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <atomic>
#include <cmath>
#include "Common/Cpp/Concurrency/Mutex.h"
#include "Common/Cpp/Containers/AlignedVector.tpp"
#include "Kernels_KaiserWindow.h"

namespace PokemonAutomation{
namespace Kernels{
namespace KaiserWindow{


double bessel_i0(double x) {
    double sum = 1.0;
    double term = 1.0;
    double x_half = x / 2.0;
    int k = 1;
    while (term > sum * 1e-16){
        double step = x_half / k;
        term *= step * step;
        sum += term;
        k++;
    }
    return sum;
}
void make_kaiser_window(float* weights, size_t window, double beta){
    double d = 1. / bessel_i0(beta);

    size_t w = window - 1;
    for (size_t c = 0; c < window; c++){
        double t = (2. * c - w) / w;
        t *= t;
        t = 1 - t;
        t = t < 0 ? 0 : std::sqrt(t);
        t = bessel_i0(beta * t) * d;
        weights[c] = (float)t;
    }
}


constexpr int MAX_WINDOW_K = 30;

Mutex lock;
AlignedVector<float> kaiser_window_cache[MAX_WINDOW_K];
std::atomic<float*> kaiser_window_cache_ptr[MAX_WINDOW_K];


const float* get_kaiser_window_k(int window_k){
    if (window_k >= MAX_WINDOW_K){
        return nullptr;
    }

    //  Try to read cache.
    float* ptr = kaiser_window_cache_ptr[window_k].load(std::memory_order_acquire);
    if (ptr != nullptr){
        return ptr;
    }

    //  Cache miss. Acquire lock to generate the cache.
    std::lock_guard<Mutex> lg(lock);

    //  Check again after acquiring lock.
    ptr = kaiser_window_cache_ptr[window_k].load(std::memory_order_acquire);
    if (ptr != nullptr){
        return ptr;
    }

    //  Now we need to generate it for real.

    size_t window = (size_t)1 << window_k;

    AlignedVector<float>& slot = kaiser_window_cache[window_k];
    slot = AlignedVector<float>(window);
    make_kaiser_window(slot.data(), window);

    kaiser_window_cache_ptr[window_k].store(slot.data(), std::memory_order_release);
    return slot.data();
}





}
}
}
