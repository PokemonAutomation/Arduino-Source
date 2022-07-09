/*  JSON Array
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Common_Json_JsonArray_H
#define PokemonAutomation_Common_Json_JsonArray_H

#include <vector>
#include "JsonValue.h"

namespace PokemonAutomation{



class JsonArray2{
public:
    JsonArray2() = default;
    JsonArray2(JsonArray2&&) = default;
    JsonArray2& operator=(JsonArray2&&) = delete;
    JsonArray2(const JsonArray2&) = delete;
    void operator=(const JsonArray2&) = delete;

public:
    std::string dump(int indent = 4) const;
    void dump(const std::string& filename, int indent = 4) const;

    bool    empty   () const{ return m_data.empty(); }
    size_t  size    () const{return m_data.size(); }

    const JsonValue2& operator[](size_t index) const{ return m_data[index]; }
          JsonValue2& operator[](size_t index)      { return m_data[index]; }

    void push_back(JsonValue2&& x){ m_data.emplace_back(std::move(x)); }

public:
    using const_iterator = std::vector<JsonValue2>::const_iterator;
    using iterator = std::vector<JsonValue2>::iterator;

    const_iterator cbegin   () const{ return m_data.cbegin(); }
    const_iterator begin    () const{ return m_data.begin(); }
          iterator begin    ()      { return m_data.begin(); }

    const_iterator cend () const{ return m_data.cend(); }
    const_iterator end  () const{ return m_data.end(); }
          iterator end  ()      { return m_data.end(); }

private:
    friend class JsonValue2;
    std::vector<JsonValue2> m_data;
};



}
#endif
