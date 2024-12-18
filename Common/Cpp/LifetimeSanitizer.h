/*  Lifetime Sanitizer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_LifetimeSanitizer_H
#define PokemonAutomation_LifetimeSanitizer_H

#include <stdint.h>
#include <atomic>
#include <string>
#include "Common/Compiler.h"

#define PA_SANITIZER_ENABLE

namespace PokemonAutomation{


#ifdef PA_SANITIZER_ENABLE
extern std::atomic<bool> LifetimeSanitizer_enabled;


class LifetimeSanitizer{
    static constexpr uint64_t SANITIZER_TOKEN = 0x7db76f7a6a834ef0;


public:
    //  Default + Destruct

    LifetimeSanitizer(const char* name = "(unnamed class)")
        : m_token(SANITIZER_TOKEN)
        , m_self(this)
        , m_name(name)
    {
        if (!LifetimeSanitizer_enabled.load(std::memory_order_relaxed)){
            return;
        }
        internal_construct();
    }
    ~LifetimeSanitizer(){
        if (!LifetimeSanitizer_enabled.load(std::memory_order_relaxed)){
            m_self = nullptr;
            return;
        }
        internal_destruct();
    }


public:
    //  Move

    LifetimeSanitizer(LifetimeSanitizer&& x)
        : m_token(SANITIZER_TOKEN)
        , m_self(this)
        , m_name(x.m_name)
    {
        if (!LifetimeSanitizer_enabled.load(std::memory_order_relaxed)){
            return;
        }
        x.check_usage();
        internal_construct();
    }
    void operator=(LifetimeSanitizer&& x){
        if (!LifetimeSanitizer_enabled.load(std::memory_order_relaxed)){
            return;
        }
        check_usage();
        x.check_usage();
    }


public:
    //  Copy

    LifetimeSanitizer(const LifetimeSanitizer& x)
        : m_token(SANITIZER_TOKEN)
        , m_self(this)
        , m_name(x.m_name)
    {
        if (!LifetimeSanitizer_enabled.load(std::memory_order_relaxed)){
            return;
        }
        x.check_usage();
        internal_construct();
    }
    void operator=(const LifetimeSanitizer& x){
        check_usage();
        x.check_usage();
    }


public:
    //  Misc.

    class CheckScope{
    public:
        CheckScope(const LifetimeSanitizer& parent)
            : m_parent(parent)
        {
            parent.check_usage();
        }
        ~CheckScope(){
            m_parent.check_usage();
        }

    private:
        const LifetimeSanitizer& m_parent;
    };

    void check_usage() const{
        if (!LifetimeSanitizer_enabled.load(std::memory_order_relaxed)){
            return;
        }
        internal_check_usage();
    }
    CheckScope check_scope() const{
        return CheckScope(*this);
    }

    static void set_enabled(bool enabled);
    static void terminate_with_dump();



private:
    PA_NO_INLINE void internal_construct();
    PA_NO_INLINE void internal_destruct();
    PA_NO_INLINE void internal_check_usage() const;

private:
    uint64_t m_token;
    void* m_self;
    std::string m_name;
};


#else
class LifetimeSanitizer{
public:
    void check_usage() const{}
    static void set_enabled(bool enabled){}
};
#endif



}
#endif
