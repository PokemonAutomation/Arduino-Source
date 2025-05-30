/*  Lifetime Sanitizer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_LifetimeSanitizer_H
#define PokemonAutomation_LifetimeSanitizer_H

#include <stdint.h>
#include <cstddef>

#define PA_SANITIZER_ENABLE

namespace PokemonAutomation{


#ifdef PA_SANITIZER_ENABLE


class LifetimeSanitizer{
    static constexpr uint64_t SANITIZER_TOKEN = 0x7db76f7a6a834ef0;


public:
    //  Rule of 5

    ~LifetimeSanitizer();
    LifetimeSanitizer(LifetimeSanitizer&& x);
    void operator=(LifetimeSanitizer&& x);
    LifetimeSanitizer(const LifetimeSanitizer& x);
    void operator=(const LifetimeSanitizer& x);


public:
    //  Constructor

    LifetimeSanitizer(const char* name = "(unnamed class)");


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

    void check_usage() const;
    void start_using() const;
    void done_using() const;
    CheckScope check_scope() const{
        return CheckScope(*this);
    }

    static void disable();


private:
    static void terminate_with_dump();
    void internal_construct(const char* name);
    void internal_destruct();


private:
    uint64_t m_token;
    void* m_self;
    const char* m_name;
    mutable size_t m_use_counter = 0;
};


#else
class LifetimeSanitizer{
public:
    LifetimeSanitizer(const char* = ""){}

    void check_usage() const{}
    static void disable(){}

    struct CheckScope{};
    CheckScope check_scope() const{
        return CheckScope();
    }
};
#endif



}
#endif
