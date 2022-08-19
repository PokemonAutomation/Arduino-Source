/*  Enum Database
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      A database that goes with an enum.
 *
 *  This database contains information about the enum such as the display names
 *  and serialization slugs.
 *
 *  This database is used to initialize an EnumDropdownCell/Option.
 *
 */

#ifndef PokemonAutomation_EnumDatabase_H
#define PokemonAutomation_EnumDatabase_H

#include <string>
#include "Containers/Pimpl.h"
#include "Containers/FixedLimitVector.h"

namespace PokemonAutomation{

class IntegerEnumDatabaseImpl;




struct EnumEntry{
    size_t enum_value;  //  Integer value of the enum.
    std::string slug;
    std::string display;
    bool enabled = true;
};


//  This is the typeless database that uses an integer for the enum value.
class IntegerEnumDatabase{
public:
    IntegerEnumDatabase(IntegerEnumDatabase&& x);
    IntegerEnumDatabase& operator=(IntegerEnumDatabase&& x);
    IntegerEnumDatabase(const IntegerEnumDatabase& x) = delete;
    IntegerEnumDatabase& operator=(const IntegerEnumDatabase& x) = delete;
    ~IntegerEnumDatabase();

public:
    IntegerEnumDatabase();      //  Constructs empty database.
    IntegerEnumDatabase(std::initializer_list<EnumEntry> list);

    void add(EnumEntry entry);
    void add(size_t value, std::string slug, std::string display, bool enabled){
        add(EnumEntry{value, std::move(slug), std::move(display), enabled});
    }

    const EnumEntry* find(size_t value) const;
    const EnumEntry* find_slug(const std::string& slug) const;
    const EnumEntry* find_display(const std::string& display) const;

    FixedLimitVector<size_t> all_values() const;

protected:
    IntegerEnumDatabase(void*); //  Constructs null database.
private:
    Pimpl<IntegerEnumDatabaseImpl> m_core;
};



//  This is the type-safe database for your specific enum.
template <typename EnumType>
class EnumDatabase : public IntegerEnumDatabase{
public:
    struct Entry{
        EnumType value;
        std::string slug;
        std::string display;
        bool enabled = true;
    };

//public:
//    EnumDatabase(EnumDatabase&& x) = default;
//    EnumDatabase& operator=(EnumDatabase&& x) = default;

public:
    EnumDatabase() : IntegerEnumDatabase() {}
    EnumDatabase(std::initializer_list<Entry> list){
        size_t index = 0;
        for (auto iter = list.begin(); iter != list.end(); ++iter, index++){
            add(iter->value, std::move(iter->slug), std::move(iter->display), iter->enabled);
        }
    }

    void add(EnumType value, std::string slug, std::string display, bool enabled){
        IntegerEnumDatabase::add(EnumEntry{(size_t)value, std::move(slug), std::move(display), enabled});
    }

    const EnumEntry* find(EnumType value) const{
        return IntegerEnumDatabase::find((size_t)value);
    }
    EnumType find_slug(const std::string& slug) const{
        return (EnumType)IntegerEnumDatabase::find_slug(slug);
    }
    EnumType find_display(const std::string& display) const{
        return (EnumType)IntegerEnumDatabase::find_display(display);
    }

protected:
    EnumDatabase(void*) : IntegerEnumDatabase(nullptr) {}
};





}
#endif
