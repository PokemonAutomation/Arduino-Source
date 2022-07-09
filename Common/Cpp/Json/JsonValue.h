/*  JSON Value
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Common_Json_JsonValue_H
#define PokemonAutomation_Common_Json_JsonValue_H

#include <stdint.h>
#include <algorithm>
#include <limits>
#include <type_traits>
#include <string>

namespace PokemonAutomation{


enum class JsonType{
    EMPTY,
    BOOLEAN,
    INTEGER,
    FLOAT,
    STRING,
    ARRAY,
    OBJECT,
};


struct JsonNode;
class JsonValue;
class JsonArray;
class JsonObject;


class JsonValue{
public:
    ~JsonValue();
    JsonValue(JsonValue&& x);
    void operator=(JsonValue&& x);
private:
    //  Private to avoid accidental copying.
    JsonValue(const JsonValue& x);
    void operator=(const JsonValue& x);
public:
    JsonValue clone() const;

public:
    JsonValue() = default;
    JsonValue(bool x);
    JsonValue(int64_t x);
    JsonValue(double x);
    JsonValue(const char* x);
    JsonValue(std::string x);
    JsonValue(JsonArray&& x);
    JsonValue(JsonObject&& x);

    template <typename Type>
    JsonValue(const Type*) = delete;

    template <typename Type, class = typename std::enable_if<std::is_integral<Type>::value>::type>
    JsonValue(Type x)
        : JsonValue((int64_t)x)
    {}

    std::string dump(int indent = 4) const;
    void dump(const std::string& filename, int indent = 4) const;

    JsonType type   () const{ return m_type; }
    bool is_null    () const{ return m_type == JsonType::EMPTY; }
    bool is_boolean () const{ return m_type == JsonType::BOOLEAN; }
    bool is_integer () const{ return m_type == JsonType::INTEGER; }
    bool is_float   () const{ return m_type == JsonType::INTEGER || m_type == JsonType::FLOAT; }
    bool is_string  () const{ return m_type == JsonType::STRING; }
    bool is_array   () const{ return m_type == JsonType::ARRAY; }
    bool is_object  () const{ return m_type == JsonType::OBJECT; }

    //  Attempt to read this value as a specific type.
    //  If the type matches, the value is assigned to "value" and returns true.
    //  Otherwise returns false and "value" remains unchanged.
    bool read_boolean(bool& value) const;
    bool read_integer(int64_t& value) const;
    bool read_integer(uint64_t& value) const;
    bool read_float(double& value) const;
    bool read_string(std::string& value) const;

    //  Same as the above, but will saturate the value to the specific min/max.
    template <typename Type>
    bool read_integer(
        Type& value,
        int64_t min = std::numeric_limits<Type>::min(),
        int64_t max = std::numeric_limits<Type>::max()
    ) const;

    //  Get a pointer to the data for this value.
    //  If the type matches, returns the pointer.
    //  If the type does match, returns nullptr.
    const std::string* get_string() const;
          std::string* get_string();
    const JsonArray* get_array() const;
          JsonArray* get_array();
    const JsonObject* get_object() const;
          JsonObject* get_object();

private:
    JsonType m_type = JsonType::EMPTY;
    JsonNode* m_node = nullptr;
};

JsonValue parse_json(const std::string& str);
JsonValue load_json_file(const std::string& str);


template <typename Type>
bool JsonValue::read_integer(Type& value, int64_t min, int64_t max) const{
    static_assert(std::is_integral<Type>::value);
    if (std::is_unsigned<Type>::value){
        uint64_t tmp;
        bool ret = read_integer(tmp);
        if (!ret){
            return false;
        }
        tmp = std::max<uint64_t>(tmp, min);
        tmp = std::min<uint64_t>(tmp, max);
        value = (Type)tmp;
    }else{
        int64_t tmp;
        bool ret = read_integer(tmp);
        if (!ret){
            return false;
        }
        tmp = std::max<int64_t>(tmp, min);
        tmp = std::min<int64_t>(tmp, max);
        value = (Type)tmp;
    }
    return true;
}






}
#endif
