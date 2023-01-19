/*  Lifetime Sanitizer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <set>
#include <atomic>
#include <iostream>
#include "Concurrency/SpinLock.h"
#include "LifetimeSanitizer.h"

//#define PA_SANITIZER_PRINT_ALL

namespace PokemonAutomation{


#ifdef PA_SANITIZER_ENABLE

SpinLock sanitizer_lock;
std::set<const LifetimeSanitizer*> sanitizer_map;


std::atomic<bool> LifetimeSanitizer_enabled(true);
bool LifetimeSanitizer_has_been_disabled = false;

void LifetimeSanitizer::set_enabled(bool enabled){
    if (enabled){
        LifetimeSanitizer_enabled.store(true, std::memory_order_relaxed);
        return;
    }
    SpinLockGuard lg(sanitizer_lock);
    LifetimeSanitizer_has_been_disabled = true;
    LifetimeSanitizer_enabled.store(false, std::memory_order_relaxed);
}





LifetimeSanitizer::LifetimeSanitizer(){
    if (!LifetimeSanitizer_enabled.load(std::memory_order_relaxed)){
        return;
    }

    SpinLockGuard lg(sanitizer_lock);
#ifdef PA_SANITIZER_PRINT_ALL
    std::cout << "LifetimeSanitizer - Allocating: " << this << std::endl;
#endif
    auto iter = sanitizer_map.find(this);
    if (iter == sanitizer_map.end()){
        sanitizer_map.insert(this);
        return;
    }
    std::cerr << "LifetimeSanitizer - Double allocation: " << this << std::endl;
    std::terminate();
}
LifetimeSanitizer::~LifetimeSanitizer(){
    if (!LifetimeSanitizer_enabled.load(std::memory_order_relaxed)){
        return;
    }

    SpinLockGuard lg(sanitizer_lock);
#ifdef PA_SANITIZER_PRINT_ALL
    std::cout << "LifetimeSanitizer - Freeing: " << this << std::endl;
#endif
    auto iter = sanitizer_map.find(this);
    if (iter != sanitizer_map.end()){
        sanitizer_map.erase(this);
        return;
    }

    //  Skip this check if we've been disabled before in case there's stuff we
    //  haven't tracked.
    if (LifetimeSanitizer_has_been_disabled){
        return;
    }

    std::cerr << "LifetimeSanitizer - Free non-existant: " << this << std::endl;
    std::terminate();
}



LifetimeSanitizer::LifetimeSanitizer(LifetimeSanitizer&& x){
    if (!LifetimeSanitizer_enabled.load(std::memory_order_relaxed)){
        return;
    }

    x.check_usage();
    SpinLockGuard lg(sanitizer_lock);
#ifdef PA_SANITIZER_PRINT_ALL
    std::cout << "LifetimeSanitizer - Allocating (move-construct): " << this << std::endl;
#endif
    auto iter = sanitizer_map.find(this);
    if (iter == sanitizer_map.end()){
        sanitizer_map.insert(this);
        return;
    }
    std::cerr << "LifetimeSanitizer - Double allocation: " << this << std::endl;
    std::terminate();
}
void LifetimeSanitizer::operator=(LifetimeSanitizer&& x){
    if (!LifetimeSanitizer_enabled.load(std::memory_order_relaxed)){
        return;
    }

    check_usage();
    x.check_usage();
}



LifetimeSanitizer::LifetimeSanitizer(const LifetimeSanitizer& x){
    if (!LifetimeSanitizer_enabled.load(std::memory_order_relaxed)){
        return;
    }

    x.check_usage();
    SpinLockGuard lg(sanitizer_lock);
#ifdef PA_SANITIZER_PRINT_ALL
    std::cout << "LifetimeSanitizer - Allocating (copy-construct): " << this << std::endl;
#endif
    auto iter = sanitizer_map.find(this);
    if (iter == sanitizer_map.end()){
        sanitizer_map.insert(this);
        return;
    }
    std::cerr << "LifetimeSanitizer - Double allocation: " << this << std::endl;
    std::terminate();
}
void LifetimeSanitizer::operator=(const LifetimeSanitizer& x){
    if (!LifetimeSanitizer_enabled.load(std::memory_order_relaxed)){
        return;
    }

    check_usage();
    x.check_usage();
}



void LifetimeSanitizer::check_usage() const{
    if (!LifetimeSanitizer_enabled.load(std::memory_order_relaxed)){
        return;
    }

    SpinLockGuard lg(sanitizer_lock);
#ifdef PA_SANITIZER_PRINT_ALL
    std::cout << "LifetimeSanitizer - Using: " << this << std::endl;
#endif
    auto iter = sanitizer_map.find(this);
    if (iter != sanitizer_map.end()){
        return;
    }
    std::cerr << "Use non-existant: " << this << std::endl;
    std::terminate();
}

#endif


}
