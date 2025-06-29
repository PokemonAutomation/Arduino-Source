/*  Enum Database
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_EnumStringMap_H
#define PokemonAutomation_EnumStringMap_H

#include <string>
#include <initializer_list>
#include <map>
#include "Common/Cpp/Exceptions.h"

namespace PokemonAutomation{



template <typename EnumType>
class EnumStringMap{
public:
    EnumStringMap(std::initializer_list<std::pair<const EnumType, std::string>> x)
        : m_enum_to_string(std::move(x))
    {
        for (const auto& item : m_enum_to_string){
            auto ret =  m_string_to_enum.emplace(item.second, item.first);
            if (!ret.second){
                throw InternalProgramError(
                    nullptr, PA_CURRENT_FUNCTION,
                    "Duplicate Enum String: " + item.second
                );
            }
        }
    }

    const std::string& get_string(EnumType type) const{
        auto iter = m_enum_to_string.find(type);
        if (iter == m_enum_to_string.end()){
            throw ParseException("Invalid Enum: " + std::to_string((uint64_t)type));
        }
        return iter->second;
    }
    EnumType get_enum(const std::string& str) const{
        auto iter = m_string_to_enum.find(str);
        if (iter == m_string_to_enum.end()){
            throw ParseException("Invalid Enum String: " + str);
        }
        return iter->second;
    }
    EnumType get_enum(const std::string& str, EnumType default_value) const{
        auto iter = m_string_to_enum.find(str);
        return iter == m_string_to_enum.end() ? default_value : iter->second;
    }

    auto begin() const{
        return m_enum_to_string.begin();
    }
    auto end() const{
        return m_enum_to_string.end();
    }

private:
    std::map<EnumType, std::string> m_enum_to_string;
    std::map<std::string, EnumType> m_string_to_enum;
};



}
#endif
