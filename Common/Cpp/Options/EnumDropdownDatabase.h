/*  Enum Database
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *      A database that goes with an enum.
 *
 *  This database contains information about the enum such as the display names
 *  and serialization slugs.
 *
 *  This database is used to initialize an EnumDropdownCell/Option.
 *
 */

#ifndef PokemonAutomation_Options_EnumDropdownDatabase_H
#define PokemonAutomation_Options_EnumDropdownDatabase_H

#include <string>
#include <vector>
#include "Common/Cpp/Containers/Pimpl.h"
#include "Common/Cpp/Containers/FixedLimitVector.h"

namespace PokemonAutomation{

class IntegerEnumDropdownDatabaseImpl;




struct EnumEntry{
    size_t enum_value;  //  Integer value of the enum.
    std::string slug;
    std::string display;
    bool enabled = true;
};


//  This is the typeless database that uses an integer for the enum value.
class IntegerEnumDropdownDatabase{
public:
    IntegerEnumDropdownDatabase(IntegerEnumDropdownDatabase&& x);
    IntegerEnumDropdownDatabase& operator=(IntegerEnumDropdownDatabase&& x);
    IntegerEnumDropdownDatabase(const IntegerEnumDropdownDatabase& x) = delete;
    IntegerEnumDropdownDatabase& operator=(const IntegerEnumDropdownDatabase& x) = delete;
    ~IntegerEnumDropdownDatabase();

public:
    IntegerEnumDropdownDatabase();      //  Constructs empty database.
    IntegerEnumDropdownDatabase(std::initializer_list<EnumEntry> list);

    void add(EnumEntry entry);
    void add(size_t value, std::string slug, std::string display, bool enabled = true){
        add(EnumEntry{value, std::move(slug), std::move(display), enabled});
    }

    //  Find an enum. Returns null if not in the database.
    const EnumEntry* find(size_t value) const;
    const EnumEntry* find_slug(const std::string& slug) const;
    const EnumEntry* find_display(const std::string& display) const;

    //  Returns the integer enum values of everything in the database.
    FixedLimitVector<size_t> all_values() const;

protected:
    IntegerEnumDropdownDatabase(void*); //  Constructs null database.
private:
    Pimpl<IntegerEnumDropdownDatabaseImpl> m_core;
};

// Create a simple StringSelectDatabase from a list of slugs.
// The display names of each entry will be the same as their slugs and enum_values will be their indices.
IntegerEnumDropdownDatabase create_integer_enum_dropdown_database(const std::vector<std::string>& slugs);



//  This is the type-safe database for your specific enum.
template <typename EnumType>
class EnumDropdownDatabase : public IntegerEnumDropdownDatabase{
public:
    struct Entry{
        EnumType value;
        std::string slug;
        std::string display;
        bool enabled = true;
    };

//public:
//    EnumDropdownDatabase(EnumDropdownDatabase&& x) = default;
//    EnumDropdownDatabase& operator=(EnumDropdownDatabase&& x) = default;

public:
    EnumDropdownDatabase() : IntegerEnumDropdownDatabase() {}
    EnumDropdownDatabase(std::initializer_list<Entry> list){
        size_t index = 0;
        for (auto iter = list.begin(); iter != list.end(); ++iter, index++){
            add(iter->value, std::move(iter->slug), std::move(iter->display), iter->enabled);
        }
    }

    void add(EnumType value, std::string slug, std::string display, bool enabled){
        IntegerEnumDropdownDatabase::add(EnumEntry{(size_t)value, std::move(slug), std::move(display), enabled});
    }

    //  Find an enum. Returns null if not in the database.
    const EnumEntry* find(EnumType value) const{
        return IntegerEnumDropdownDatabase::find((size_t)value);
    }
    EnumType find_slug(const std::string& slug) const{
        return (EnumType)IntegerEnumDropdownDatabase::find_slug(slug);
    }
    EnumType find_display(const std::string& display) const{
        return (EnumType)IntegerEnumDropdownDatabase::find_display(display);
    }

protected:
    EnumDropdownDatabase(void*) : IntegerEnumDropdownDatabase(nullptr) {}
};





}
#endif
