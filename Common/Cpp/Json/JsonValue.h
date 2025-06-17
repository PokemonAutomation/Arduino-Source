/*  JSON Value
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Common_Json_JsonValue_H
#define PokemonAutomation_Common_Json_JsonValue_H

#include <stdint.h>
#include <algorithm>
#include <limits>
#include <type_traits>
#include <string>
#include "Common/Cpp/Exceptions.h"

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
const std::string& get_typename(JsonType type);


class JsonParseException : public ParseException{
public:
    JsonParseException( //  Mismatching Type
        const std::string& filename,
        JsonType expected_type,
        JsonType actual_type
    );
    JsonParseException( //  Out-of-bounds index.
        const std::string& filename,
        size_t size, size_t index
    );
    JsonParseException( //  Missing Key
        const std::string& filename,
        const std::string& key
    );
    JsonParseException( //  Mismatching Type through an object.
        const std::string& filename,
        const std::string& key,
        JsonType expected_type,
        JsonType actual_type
    );
    virtual const char* name() const override{ return "JsonParseException"; }
    virtual std::string message() const override{ return m_message; }
};


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

    void clear();

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

    //  Get the value. Throws if the type doesn't match.
    bool                to_boolean_throw(const std::string& filename = std::string()) const;
    int64_t             to_integer_throw(const std::string& filename = std::string()) const;
    double              to_double_throw (const std::string& filename = std::string()) const;
    const std::string&  to_string_throw (const std::string& filename = std::string()) const;
          std::string&  to_string_throw (const std::string& filename = std::string());
    const JsonArray&    to_array_throw  (const std::string& filename = std::string()) const;
          JsonArray&    to_array_throw  (const std::string& filename = std::string());
    const JsonObject&   to_object_throw (const std::string& filename = std::string()) const;
          JsonObject&   to_object_throw (const std::string& filename = std::string());

    //  Get a pointer to the data for this value.
    //  If the type matches, returns the pointer.
    //  If the type does match, returns nullptr.
    const std::string*  to_string   () const;
          std::string*  to_string   ();
    const JsonArray*    to_array    () const;
          JsonArray*    to_array    ();
    const JsonObject*   to_object   () const;
          JsonObject*   to_object   ();

    //  Convert to the specified type. If the type doesn't match, return the default.
    bool        to_boolean_default (bool default_value = false) const;
    int64_t     to_integer_default (int64_t default_value = 0) const;
    double      to_double_default  (double default_value = 0) const;
    std::string to_string_default  (const char* default_value = "") const;

    //  Attempt to read this value as a specific type.
    //  If the type matches, the value is assigned to "value" and returns true.
    //  Otherwise returns false and "value" remains unchanged.
    bool read_boolean   (bool& value) const;
    bool read_integer   (int64_t& value) const;
    bool read_integer   (uint64_t& value) const;
    bool read_float     (double& value) const;
    bool read_string    (std::string& value) const;

    //  Same as the above, but will saturate the value to the specific min/max.
    template <typename Type>
    bool read_integer(
        Type& value,
        int64_t min = std::numeric_limits<Type>::min(),
        int64_t max = std::numeric_limits<Type>::max()
    ) const;

private:
    JsonType m_type = JsonType::EMPTY;
    union{
        bool m_bool;
        int64_t m_integer;
        double m_float;
        std::string* m_string;
        JsonArray* m_array;
        JsonObject* m_object;
    } u;
};

// Given a string as a raw JSON, parse it into a `JsonValue`.
// If there is error parsing the JSON, it will not throw exception.
// The input string is usually loaded directly from a JSON file.
// You can call JsonTools.h:file_to_string() to load a file as a raw JSON string.
JsonValue parse_json(const std::string& str);
// Load file from `filename` and parse it into a `JsonValue`.
// If unable to open the file, FileException is thrown
// If there is error parsing the JSON, it will not throw exception.
JsonValue load_json_file(const std::string& filename);


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
