/*  ScopeExit
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_ScopeExit_h
#define PokemonAutomation_ScopeExit_h

#include <functional>

namespace PokemonAutomation{
class ScopeExit{
    ScopeExit(const ScopeExit&) = delete;
    void operator=(const ScopeExit&) = delete;

public:
    template <typename Lambda>
    ScopeExit(Lambda&& lambda)
        : m_lambda(std::move(lambda))
    {}
    ~ScopeExit(){
        m_lambda();
    }

private:
    std::function<void()> m_lambda;
};

}
#endif