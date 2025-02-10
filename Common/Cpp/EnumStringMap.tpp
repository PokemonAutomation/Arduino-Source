/*  Enum Database
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <map>
#include "Common/Cpp/Exceptions.h"
#include "Containers/Pimpl.tpp"
#include "EnumStringMap.h"

namespace PokemonAutomation{




template <typename EnumType>
struct EnumStringMap<EnumType>::Data{
    std::map<EnumType, std::string> enum_to_string;
    std::map<std::string, EnumType> string_to_enum;
};



template <typename EnumType>
EnumStringMap<EnumType>::EnumStringMap(std::initializer_list<std::pair<const EnumType, std::string>> x)
    : m_data(CONSTRUCT_TOKEN, std::move(x))
{
    for (const auto& item : m_data->enum_to_string){
        auto ret =  m_data->string_to_enum.emplace(item.second, item.first);
        if (!ret.second){
            throw InternalProgramError(
                nullptr, PA_CURRENT_FUNCTION,
                "Duplicate Enum String: " + item.second
            );
        }
    }
}
template <typename EnumType>
EnumStringMap<EnumType>::~EnumStringMap() = default;



template <typename EnumType>
const std::string& EnumStringMap<EnumType>::get(EnumType type){
    auto iter = m_data->enum_to_string.find(type);
    if (iter == m_data->enum_to_string.end()){
        throw ParseException("Invalid Enum: " + std::to_string((uint64_t)type));
    }
    return iter->second;
}
template <typename EnumType>
EnumType EnumStringMap<EnumType>::get(const std::string& str){
    auto iter = m_data->string_to_enum.find(str);
    if (iter == m_data->string_to_enum.end()){
        throw ParseException("Invalid Enum String: " + str);
    }
    return iter->second;
}
template <typename EnumType>
EnumType EnumStringMap<EnumType>::get(const std::string& str, EnumType default_value){
    auto iter = m_data->string_to_enum.find(str);
    return iter == m_data->string_to_enum.end() ? default_value : iter->second;
}




}
