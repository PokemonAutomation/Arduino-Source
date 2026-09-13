/*  Multi-Console System Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_GameConsole_MultiConsoleSystemOption_H
#define PokemonAutomation_GameConsole_MultiConsoleSystemOption_H

#include <functional>
#include "Common/Cpp/Containers/FixedLimitVector.h"
#include "ConsoleSystemOption.h"

namespace PokemonAutomation{
namespace GameConsole{


class MultiConsoleSystemOption{
public:
    static const size_t MAX_CONSOLES = 4;

public:
    virtual ~MultiConsoleSystemOption();
    MultiConsoleSystemOption(
        size_t min_consoles,
        size_t max_consoles,
        size_t consoles,
        std::function<std::unique_ptr<ConsoleSystemOption>()> factory = []{
            return std::make_unique<ConsoleSystemOption>(1);
        }
    );
    MultiConsoleSystemOption(
        size_t min_consoles,
        size_t max_consoles,
        const JsonValue& json,
        std::function<std::unique_ptr<ConsoleSystemOption>()> factory = []{
            return std::make_unique<ConsoleSystemOption>(1);
        }
    );

    void resize(size_t count){
        m_active_consoles = count;
    }

public:
    size_t min_consoles() const{ return m_min_consoles; }
    size_t max_consoles() const{ return m_max_consoles; }

    size_t active_consoles() const{ return m_active_consoles; }
    ConsoleSystemOption& operator[](size_t index){ return *m_consoles[index]; }


public:
    virtual JsonValue to_json() const;
    virtual void load_json(const JsonValue& json);


private:
    const size_t m_min_consoles;
    const size_t m_max_consoles;
    size_t m_active_consoles;
    FixedLimitVector<std::unique_ptr<ConsoleSystemOption>> m_consoles;
};




}
}
#endif
