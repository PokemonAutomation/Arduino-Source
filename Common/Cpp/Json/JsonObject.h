/*  JSON Object
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Common_Json_JsonObject_H
#define PokemonAutomation_Common_Json_JsonObject_H

#include <map>
#include "JsonValue.h"

namespace PokemonAutomation{


class JsonObject2{
public:
    JsonObject2() = default;
    JsonObject2(JsonObject2&&) = default;
    JsonObject2& operator=(JsonObject2&&) = delete;
    JsonObject2(const JsonObject2&) = delete;
    void operator=(const JsonObject2&) = delete;

public:
    std::string dump(int indent = 4) const;
    void dump(const std::string& filename, int indent = 4) const;

    bool    empty   () const{ return m_data.empty(); }
    size_t  size    () const{return m_data.size(); }

    JsonValue2& operator[](const std::string& key){ return m_data[key]; }
    JsonValue2& operator[](     std::string&& key){ return m_data[std::move(key)]; }

    bool read_boolean(bool& value, const std::string& key) const;
    bool read_float(double& value, const std::string& key) const;
    bool read_string(std::string& value, const std::string& key) const;

    template <typename Type>
    bool read_integer(
        Type& value, const std::string& key,
        int64_t min = std::numeric_limits<Type>::min(),
        int64_t max = std::numeric_limits<Type>::max()
    ) const;

    const std::string* get_string(const std::string& key) const;
          std::string* get_string(const std::string& key);
    const JsonArray2* get_array(const std::string& key) const;
          JsonArray2* get_array(const std::string& key);
    const JsonObject2* get_object(const std::string& key) const;
          JsonObject2* get_object(const std::string& key);
    const JsonValue2* get_value(const std::string& key) const;
          JsonValue2* get_value(const std::string& key);

public:
    using const_iterator = std::map<std::string, JsonValue2>::const_iterator;
    using iterator = std::map<std::string, JsonValue2>::iterator;

    const_iterator find(const std::string& key) const{ return m_data.find(key); }
          iterator find(const std::string& key)      { return m_data.find(key); }

    const_iterator cbegin   () const{ return m_data.cbegin(); }
    const_iterator begin    () const{ return m_data.begin(); }
          iterator begin    ()      { return m_data.begin(); }

    const_iterator cend () const{ return m_data.cend(); }
    const_iterator end  () const{ return m_data.end(); }
          iterator end  ()      { return m_data.end(); }

private:
    friend class JsonValue2;
    std::map<std::string, JsonValue2> m_data;
};



template <typename Type>
bool JsonObject2::read_integer(Type& value, const std::string& key, int64_t min, int64_t max) const{
    static_assert(std::is_integral<Type>::value);
    auto iter = m_data.find(key);
    if (iter == m_data.end()){
        return false;
    }
    return iter->second.read_integer(value, min, max);
}




}
#endif
