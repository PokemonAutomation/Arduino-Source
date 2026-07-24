/*  Unit Test
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_UnitTest_H
#define PokemonAutomation_UnitTest_H

#include <memory>
#include <string>

namespace PokemonAutomation{

class CancellableScope;



struct UnitTestResult{
    enum Result{
        NOT_RUN,
        PASSED,
        FAILED,
        SKIPPED,
        OOM,
    };
    Result result;
    std::string message;

    UnitTestResult()
        : result(NOT_RUN)
    {}
    UnitTestResult(bool pass)
        : result(pass ? PASSED : FAILED)
    {}
    UnitTestResult(std::string p_message)
        : result(FAILED)
        , message(std::move(p_message))
    {}
    UnitTestResult(Result p_result, std::string p_message = "")
        : result(p_result)
        , message(std::move(p_message))
    {}
};



class UnitTest{
public:
    virtual ~UnitTest() = default;
    UnitTest(std::string name)
        : m_name(std::move(name))
    {}


    template <typename TestType, class... Args>
    static std::unique_ptr<UnitTest> make(Args&&... args){
        return std::make_unique<TestType>(std::forward<Args>(args)...);
    }

    const std::string& name() const{
        return m_name;
    }
    uint64_t memory() const{
        return m_memory;
    }
    size_t threads() const{
        return m_threads;
    }

    virtual UnitTestResult run(CancellableScope& scope) const = 0;


protected:
    const std::string m_name;
    uint64_t m_memory = 0;
    size_t m_threads = 1;
};






}
#endif
