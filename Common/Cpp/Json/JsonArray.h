/*  JSON Array
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Common_Json_JsonArray_H
#define PokemonAutomation_Common_Json_JsonArray_H

#include <vector>
#include "JsonValue.h"

namespace PokemonAutomation{



class JsonArray{
public:
    JsonArray() = default;
    JsonArray(JsonArray&& x) = default;
    JsonArray& operator=(JsonArray&& x) = default;
private:
    //  Private to avoid accidental copying.
    JsonArray(const JsonArray& x);
    JsonArray& operator=(const JsonArray& x);
public:
    JsonArray clone() const;

public:
    std::string dump(int indent = 4) const;
    void dump(const std::string& filename, int indent = 4) const;

    bool    empty   () const{ return m_data.empty(); }
    size_t  size    () const{ return m_data.size(); }

    const JsonValue& operator[](size_t index) const;
          JsonValue& operator[](size_t index)      ;

    void push_back(JsonValue&& x){ m_data.emplace_back(std::move(x)); }

public:
    using const_iterator = std::vector<JsonValue>::const_iterator;
    using       iterator = std::vector<JsonValue>::iterator;

    const_iterator cbegin   () const{ return m_data.cbegin(); }
    const_iterator begin    () const{ return m_data.begin(); }
          iterator begin    ()      { return m_data.begin(); }

    const_iterator cend () const{ return m_data.cend(); }
    const_iterator end  () const{ return m_data.end(); }
          iterator end  ()      { return m_data.end(); }

private:
    friend class JsonValue;
    std::vector<JsonValue> m_data;
};



}
#endif
