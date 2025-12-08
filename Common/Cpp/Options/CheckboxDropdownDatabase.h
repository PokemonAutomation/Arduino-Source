/*  Checkbox Dropdown Database
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *      A database that goes with a checkbox dropdown menu.
 *
 *  This database contains information about the enum such as the display names
 *  and serialization slugs.
 *
 *  This database is used to initialize an CheckboxDropdownCell/Option.
 *
 */

#ifndef PokemonAutomation_Options_CheckboxDropdownDatabase_H
#define PokemonAutomation_Options_CheckboxDropdownDatabase_H

#include <vector>
#include <map>
#include "Common/Cpp/Exceptions.h"

namespace PokemonAutomation{



template <typename FlagEnum>
struct FlagEnumEntry{
    FlagEnum value;
    std::string slug;
    std::string display;
};


template <typename FlagEnum>
class CheckboxDropdownDatabase{
    using Entry = FlagEnumEntry<FlagEnum>;

public:
    CheckboxDropdownDatabase() = default;
    CheckboxDropdownDatabase(std::initializer_list<Entry> list){
        for (auto iter = list.begin(); iter != list.end(); ++iter){
            add(iter->value, std::move(iter->slug), std::move(iter->display));
        }
    }

    //  Warning, these functions do not have strong exception safety!
    //  If these throw, this class will be in a bad state.
    void add(Entry entry){
        Entry& e = m_list.emplace_back(std::move(entry));

        auto ret = m_slug_to_enum.emplace(e.slug, &e);
        if (!ret.second){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Duplicate Enum Slug: " + e.slug);
        }
    }
    void add(FlagEnum value, std::string slug, std::string display){
        add(Entry{value, std::move(slug), std::move(display)});
    }


public:
    size_t size() const{
        return m_list.size();
    }
    const Entry& operator[](size_t index) const{
        return m_list[index];
    }
    const FlagEnum* find_slug(const std::string& slug) const{
        auto iter = m_slug_to_enum.find(slug);
        if (iter == m_slug_to_enum.end()){
            return nullptr;
        }
        return &iter->second->value;
    }

public:
    auto begin() const{
        return m_list.begin();
    }
    auto end() const{
        return m_list.end();
    }


private:
    std::vector<Entry> m_list;
    std::map<std::string, const Entry*> m_slug_to_enum;
};




}
#endif
