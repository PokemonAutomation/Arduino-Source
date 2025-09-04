/*  Lifetime Sanitizer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <set>
//#include <exception>
#include <atomic>
#include <iostream>
#include "Concurrency/SpinLock.h"
#include "LifetimeSanitizer.h"

#ifdef _WIN32
#include <Windows.h>
#endif



namespace PokemonAutomation{


#ifdef PA_SANITIZER_ENABLE

//#define PA_SANITIZER_PRINT_ALL
const std::set<std::string> SANITIZER_FILTER = {
//    "MultiSwitchProgramSession",
//    "MultiSwitchProgramWidget2",
};

SpinLock sanitizer_lock;
std::set<const LifetimeSanitizer*> sanitizer_map;


std::atomic<bool> LifetimeSanitizer_disabled(false);

void LifetimeSanitizer::disable(){
    WriteSpinLock lg(sanitizer_lock);
    LifetimeSanitizer_disabled.store(true, std::memory_order_relaxed);
    sanitizer_map.clear();
}

PA_NO_INLINE void LifetimeSanitizer::terminate_with_dump(){
    //  Intentionally crash the program here and let the crash dump deal with
    //  the error reporting.

#ifdef _WIN32
    Sleep(500);
#endif

    std::cerr << (char*)nullptr << std::endl;
}




LifetimeSanitizer::LifetimeSanitizer(const char* name){
    if (LifetimeSanitizer_disabled.load(std::memory_order_relaxed)){
        return;
    }
    internal_construct(name);
}
LifetimeSanitizer::~LifetimeSanitizer(){
    if (LifetimeSanitizer_disabled.load(std::memory_order_relaxed)){
        return;
    }
    internal_destruct();
}


LifetimeSanitizer::LifetimeSanitizer(LifetimeSanitizer&& x){
    if (LifetimeSanitizer_disabled.load(std::memory_order_relaxed)){
        return;
    }
    x.check_usage();
    internal_construct(x.m_name);
}
void LifetimeSanitizer::operator=(LifetimeSanitizer&& x){
    if (LifetimeSanitizer_disabled.load(std::memory_order_relaxed)){
        return;
    }
    check_usage();
    x.check_usage();
}
LifetimeSanitizer::LifetimeSanitizer(const LifetimeSanitizer& x){
    if (LifetimeSanitizer_disabled.load(std::memory_order_relaxed)){
        return;
    }
    x.check_usage();
    internal_construct(x.m_name);
}
void LifetimeSanitizer::operator=(const LifetimeSanitizer& x){
    if (LifetimeSanitizer_disabled.load(std::memory_order_relaxed)){
        return;
    }
    check_usage();
    x.check_usage();
}



void LifetimeSanitizer::check_usage() const{
    if (LifetimeSanitizer_disabled.load(std::memory_order_relaxed)){
        return;
    }
    ReadSpinLock lg(sanitizer_lock);
    auto iter = sanitizer_map.find(this);
    if (iter == sanitizer_map.end()){
        std::cerr << "Use non-existent: " << this << std::endl;
        terminate_with_dump();
    }
    if (SANITIZER_FILTER.contains(m_name)){
        std::cout << "LifetimeSanitizer - Using: " << this << " : " << m_name << std::endl;
    }
    if (m_token != SANITIZER_TOKEN || m_self != this){
        std::cerr << "Use corrupted: " << this << " : " << m_name << std::endl;
        terminate_with_dump();
    }
}
void LifetimeSanitizer::start_using() const{
    if (LifetimeSanitizer_disabled.load(std::memory_order_relaxed)){
        return;
    }
    ReadSpinLock lg(sanitizer_lock);
    auto iter = sanitizer_map.find(this);
    if (iter == sanitizer_map.end()){
        std::cerr << "Start using non-existent: " << this << std::endl;
        terminate_with_dump();
    }
    if (SANITIZER_FILTER.contains(m_name)){
        std::cout << "LifetimeSanitizer - Start using: " << this << " : " << m_name << std::endl;
    }
    if (m_token != SANITIZER_TOKEN || m_self != this){
        std::cerr << "Start using corrupted: " << this << " : " << m_name << std::endl;
        terminate_with_dump();
    }
    m_use_counter++;
}
void LifetimeSanitizer::done_using() const{
    if (LifetimeSanitizer_disabled.load(std::memory_order_relaxed)){
        return;
    }
    ReadSpinLock lg(sanitizer_lock);
    auto iter = sanitizer_map.find(this);
    if (iter == sanitizer_map.end()){
        std::cerr << "Done using non-existent: " << this << std::endl;
        terminate_with_dump();
    }
    if (SANITIZER_FILTER.contains(m_name)){
        std::cout << "LifetimeSanitizer - Done using: " << this << " : " << m_name << std::endl;
    }
    if (m_token != SANITIZER_TOKEN || m_self != this){
        std::cerr << "Done using corrupted: " << this << " : " << m_name << std::endl;
        terminate_with_dump();
    }
    m_use_counter--;
}


void LifetimeSanitizer::internal_construct(const char* name){
    WriteSpinLock lg(sanitizer_lock);
    if (SANITIZER_FILTER.contains(name)){
        std::cout << "LifetimeSanitizer - Allocating: " << this << " : " << name << std::endl;
    }

    auto iter = sanitizer_map.find(this);
    if (iter != sanitizer_map.end()){
        std::cerr << "LifetimeSanitizer - Double allocation: " << this << " : " << name << std::endl;
        terminate_with_dump();
    }
    sanitizer_map.insert(this);

    m_token = SANITIZER_TOKEN;
    m_self = this;
    m_name = name;
}
void LifetimeSanitizer::internal_destruct(){
    WriteSpinLock lg(sanitizer_lock);
    if (SANITIZER_FILTER.contains(m_name)){
        std::cout << "LifetimeSanitizer - Freeing: " << this << " : " << m_name << std::endl;
    }

    auto iter = sanitizer_map.find(this);
    if (iter == sanitizer_map.end()){
        std::cerr << "LifetimeSanitizer - Free non-existent: " << this << " : " << m_name << std::endl;
        terminate_with_dump();
    }
    sanitizer_map.erase(this);

    if (m_token != SANITIZER_TOKEN || m_self != this){
        std::cerr << "LifetimeSanitizer - Free non-existent: " << this << " : " << m_name << std::endl;
        terminate_with_dump();
    }
    if (m_use_counter != 0){
        std::cerr << "LifetimeSanitizer - Freeing while in-use: " << this << " : " << m_name << std::endl;
        terminate_with_dump();
    }
    m_self = nullptr;
}
#endif






}
