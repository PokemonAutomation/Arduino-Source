/*  Named Enum
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <vector>
#include <map>
#include "Exceptions.h"
#include "Containers/Pimpl.tpp"
#include "Containers/FixedLimitVector.tpp"
#include "NamedEnum.h"

namespace PokemonAutomation{



class IntegerEnumDatabaseImpl{
public:
    IntegerEnumDatabaseImpl();

    void add(EnumEntry entry){
        size_t enum_value = entry.enum_value;
        m_list.emplace_back(enum_value);

        auto ret = m_map.emplace(enum_value, std::move(entry));
        if (!ret.second){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Duplicate Enum: " + std::to_string(enum_value));
        }

        auto ret1 = m_slug_to_enum.emplace(ret.first->second.slug, &ret.first->second);
        if (!ret1.second){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Duplicate Enum Slug: " + ret.first->second.slug);
        }

        auto ret2 = m_display_to_enum.emplace(ret.first->second.display, &ret.first->second);
        if (!ret2.second){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Duplicate Enum Display: " + ret.first->second.display);
        }
    }

    const EnumEntry* find(size_t value) const{
        auto iter = m_map.find(value);
        if (iter == m_map.end()){
            return nullptr;
        }
        return &iter->second;
    }
    const EnumEntry* find_slug(const std::string& slug) const{
        auto iter = m_slug_to_enum.find(slug);
        if (iter == m_slug_to_enum.end()){
            return nullptr;
        }
        return iter->second;
    }
    const EnumEntry* find_display(const std::string& display) const{
        auto iter = m_display_to_enum.find(display);
        if (iter == m_display_to_enum.end()){
            return nullptr;
        }
        return iter->second;
    }

    FixedLimitVector<size_t> all_values() const{
        FixedLimitVector<size_t> ret(m_map.size());
        for (const auto& item : m_map){
            ret.emplace_back(item.first);
        }
        return ret;
    }

private:
    using Node = typename std::map<size_t, EnumEntry>::const_iterator;
    std::vector<size_t> m_list;
    std::map<size_t, EnumEntry> m_map;
    std::map<std::string, const EnumEntry*> m_slug_to_enum;
    std::map<std::string, const EnumEntry*> m_display_to_enum;
};




IntegerEnumDatabase::~IntegerEnumDatabase() = default;
IntegerEnumDatabase::IntegerEnumDatabase() = default;
void IntegerEnumDatabase::add(EnumEntry entry){
    m_core->add(entry);
}
const EnumEntry* IntegerEnumDatabase::find(size_t value) const{
    return m_core->find(value);
}
const EnumEntry* IntegerEnumDatabase::find_slug(const std::string& slug) const{
    return m_core->find_slug(slug);
}
const EnumEntry* IntegerEnumDatabase::find_display(const std::string& display) const{
    return m_core->find_display(display);
}
FixedLimitVector<size_t> IntegerEnumDatabase::all_values() const{
    return m_core->all_values();
}





}
