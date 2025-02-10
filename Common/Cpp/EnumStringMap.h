/*  Enum Database
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_EnumStringMap_H
#define PokemonAutomation_EnumStringMap_H

#include <string>
#include <initializer_list>
#include "Containers/Pimpl.h"

namespace PokemonAutomation{



template <typename EnumType>
class EnumStringMap{
public:
    EnumStringMap(std::initializer_list<std::pair<const EnumType, std::string>> x);
    ~EnumStringMap();

    const std::string& get(EnumType type);
    EnumType get(const std::string& str);
    EnumType get(const std::string& str, EnumType default_value);

private:
    struct Data;
    Pimpl<Data> m_data;
};



}
#endif
