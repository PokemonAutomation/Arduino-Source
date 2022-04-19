/*  Lifetime Sanitizer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <set>
#include <iostream>
#include "SpinLock.h"
#include "LifetimeSanitizer.h"

//#define PA_SANITIZER_PRINT_ALL

namespace PokemonAutomation{


#ifdef PA_SANITIZER_ENABLE

SpinLock sanitizer_lock;
std::set<const LifetimeSanitizer*> sanitizer_map;


LifetimeSanitizer::LifetimeSanitizer(){
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
    SpinLockGuard lg(sanitizer_lock);
#ifdef PA_SANITIZER_PRINT_ALL
    std::cout << "LifetimeSanitizer - Freeing: " << this << std::endl;
#endif
    auto iter = sanitizer_map.find(this);
    if (iter != sanitizer_map.end()){
        sanitizer_map.erase(this);
        return;
    }
    std::cerr << "LifetimeSanitizer -Free non-existant: " << this << std::endl;
    std::terminate();
}
void LifetimeSanitizer::check_usage() const{
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
