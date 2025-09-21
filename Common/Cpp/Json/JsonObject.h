/*  JSON Object
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Common_Json_JsonObject_H
#define PokemonAutomation_Common_Json_JsonObject_H

#include <map>
#include "JsonValue.h"

namespace PokemonAutomation{


class JsonObject{
public:
    JsonObject() = default;
    JsonObject(JsonObject&& x) = default;
    JsonObject& operator=(JsonObject&& x) = default;

    bool operator==(const JsonObject& x){ // TODO: implement == properly. For JsonValue as well.
        return dump() == x.dump();
    }
    bool operator!=(const JsonObject& x){
        return !(*this == *this);
    }
private:
    //  Private to avoid accidental copying.
    JsonObject(const JsonObject& x);
    JsonObject& operator=(const JsonObject& x);
public:
    JsonObject clone() const;

public:
    std::string dump(int indent = 4) const;
    void dump(const std::string& filename, int indent = 4) const;

    bool    empty   () const{ return m_data.empty(); }
    size_t  size    () const{ return m_data.size(); }

    JsonValue& operator[](const std::string& key){ return m_data[key]; }
    JsonValue& operator[](     std::string&& key){ return m_data[std::move(key)]; }

    //  Get the value. Throws if the type doesn't match.
    bool                get_boolean_throw   (const std::string& key, const std::string& filename = std::string()) const;
    int64_t             get_integer_throw   (const std::string& key, const std::string& filename = std::string()) const;
    double              get_double_throw    (const std::string& key, const std::string& filename = std::string()) const;
    const std::string&  get_string_throw    (const std::string& key, const std::string& filename = std::string()) const;
          std::string&  get_string_throw    (const std::string& key, const std::string& filename = std::string());
    const JsonArray&    get_array_throw     (const std::string& key, const std::string& filename = std::string()) const;
          JsonArray&    get_array_throw     (const std::string& key, const std::string& filename = std::string());
    const JsonObject&   get_object_throw    (const std::string& key, const std::string& filename = std::string()) const;
          JsonObject&   get_object_throw    (const std::string& key, const std::string& filename = std::string());
    const JsonValue&    get_value_throw     (const std::string& key, const std::string& filename = std::string()) const;
          JsonValue&    get_value_throw     (const std::string& key, const std::string& filename = std::string());

    //  Get a pointer to the value at the specified key.
    //  If the key exists and the type matches, returns the pointer.
    //  If the type does match, returns nullptr.
    const std::string*  get_string  (const std::string& key) const;
          std::string*  get_string  (const std::string& key);
    const JsonArray*    get_array   (const std::string& key) const;
          JsonArray*    get_array   (const std::string& key);
    const JsonObject*   get_object  (const std::string& key) const;
          JsonObject*   get_object  (const std::string& key);
    const JsonValue*    get_value   (const std::string& key) const;
          JsonValue*    get_value   (const std::string& key);

#if 0
    //  Convert to the specified type. If the type doesn't match, return the default.
    bool        to_boolean  (const std::string& key, bool default_value = false) const;
    int64_t     to_integer  (const std::string& key, int64_t default_value = 0) const;
    double      to_double   (const std::string& key, double default_value = 0) const;
    std::string to_string   (const std::string& key, const char* default_value = "") const;
#endif

    //  Attempt to read this value at the specified key.
    //  If the key exists and the type matches, the value is assigned to "value" and returns true.
    //  Otherwise returns false and "value" remains unchanged.
    bool read_boolean(bool& value, const std::string& key) const;
    bool read_float(double& value, const std::string& key) const;
    bool read_string(std::string& value, const std::string& key) const;

    //  Same as the above, but will saturate the value to the specific min/max.
    template <typename Type>
    bool read_integer(
        Type& value, const std::string& key,
        int64_t min = std::numeric_limits<Type>::min(),
        int64_t max = std::numeric_limits<Type>::max()
    ) const;

public:
    using const_iterator = std::map<std::string, JsonValue>::const_iterator;
    using       iterator = std::map<std::string, JsonValue>::iterator;

    const_iterator find(const std::string& key) const{ return m_data.find(key); }
          iterator find(const std::string& key)      { return m_data.find(key); }

    const_iterator cbegin   () const{ return m_data.cbegin(); }
    const_iterator begin    () const{ return m_data.begin(); }
          iterator begin    ()      { return m_data.begin(); }

    const_iterator cend () const{ return m_data.cend(); }
    const_iterator end  () const{ return m_data.end(); }
          iterator end  ()      { return m_data.end(); }

private:
    friend class JsonValue;
    std::map<std::string, JsonValue> m_data;
};



template <typename Type>
bool JsonObject::read_integer(Type& value, const std::string& key, int64_t min, int64_t max) const{
    static_assert(std::is_integral<Type>::value);
    auto iter = m_data.find(key);
    if (iter == m_data.end()){
        return false;
    }
    return iter->second.read_integer(value, min, max);
}




}
#endif
