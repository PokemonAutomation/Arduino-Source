/*  Unit Test
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_UnitTest_H
#define PokemonAutomation_UnitTest_H

#include <string>
#include "Common/Cpp/CancellableScope.h"

namespace PokemonAutomation{


enum class UnitTestResult{
    NOT_RUN,
    PASSED,
    FAILED,
    SKIPPED,
    OOM,
};


class UnitTest{
public:
    virtual ~UnitTest() = default;
    UnitTest(std::string name)
        : m_name(std::move(name))
    {}

    const std::string& name() const{
        return m_name;
    }
    uint64_t memory() const{
        return m_memory;
    }
    size_t threads() const{
        return m_threads;
    }
    UnitTestResult result() const{
        return m_result;
    }
    const std::string& message() const{
        return m_message;
    }

    virtual std::pair<UnitTestResult, std::string> run(CancellableScope& scope) = 0;


protected:
    friend class UnitTestRunner;

    const std::string m_name;
    uint64_t m_memory = 0;
    size_t m_threads = 1;

private:
    UnitTestResult m_result = UnitTestResult::NOT_RUN;
    std::string m_message;
};






}
#endif
