/*  Unit Test Database
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_UnitTestDatabase_H
#define PokemonAutomation_UnitTestDatabase_H

#include <map>
#include "Common/Cpp/Exceptions.h"
#include "UnitTest.h"

namespace PokemonAutomation{


class UnitTestDatabase{
public:
    template <typename TestType, class... Args>
    void add(Args&&... args){
        *this += std::make_shared<TestType>(std::forward<Args>(args)...);
    }
    void operator+=(std::shared_ptr<const UnitTest> test){
        if (!m_database.emplace(test->name(), std::move(test)).second){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Duplicate Test Name: " + test->name());
        }
    }
    void operator+=(const UnitTestDatabase& database){
        for (const auto& item : database.m_database){
            if (m_database.contains(item.first)){
                throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Duplicate Test Name: " + item.first);
            }
        }
        std::map<std::string, std::shared_ptr<const UnitTest>> copy = database.m_database;
        while (!copy.empty()){
            auto node = copy.extract(copy.begin());
            m_database.insert(std::move(node));
        }
    }

public:
    auto find(const std::string& name) const{
        return m_database.find(name);
    }
    auto begin() const{
        return m_database.begin();
    }
    auto end() const{
        return m_database.end();
    }

private:
    std::map<std::string, std::shared_ptr<const UnitTest>> m_database;
};



}
#endif
