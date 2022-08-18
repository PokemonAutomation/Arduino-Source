/*  Named Enum
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NamedEnum_H
#define PokemonAutomation_NamedEnum_H

#include <string>
#include "Containers/Pimpl.h"
#include "Containers/FixedLimitVector.h"

namespace PokemonAutomation{

class IntegerEnumDatabaseImpl;




struct EnumEntry{
    size_t enum_value;
    std::string slug;
    std::string display;
    bool enabled = true;
};


class IntegerEnumDatabase{
public:
    ~IntegerEnumDatabase();
    IntegerEnumDatabase();

    void add(EnumEntry entry);
    void add(size_t value, std::string slug, std::string display, bool enabled){
        add(EnumEntry{value, std::move(slug), std::move(display), enabled});
    }

    const EnumEntry* find(size_t value) const;
    const EnumEntry* find_slug(const std::string& slug) const;
    const EnumEntry* find_display(const std::string& display) const;

    FixedLimitVector<size_t> all_values() const;

private:
    Pimpl<IntegerEnumDatabaseImpl> m_core;
};




template <typename EnumType>
class NamedEnumDatabase : public IntegerEnumDatabase{
public:
    struct Entry{
        EnumType value;
        std::string slug;
        std::string display;
        bool enabled = true;
    };

public:
    NamedEnumDatabase();
    NamedEnumDatabase(std::initializer_list<Entry> list){
        size_t index = 0;
        for (auto iter = list.begin(); iter != list.end(); ++iter, index++){
            add(iter->value, std::move(iter->slug), std::move(iter->display), iter->enabled);
        }
    }

    void add(EnumType value, EnumEntry entry){
        IntegerEnumDatabase::add((size_t)value, entry);
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
};





}
#endif
