/*  Language
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Language_H
#define PokemonAutomation_Language_H

#include <string>
#include <set>

namespace PokemonAutomation{

enum class Language{
    None,
    English,
    Japanese,
    Spanish,
    French,
    German,
    Italian,
    Korean,
    ChineseSimplified,
    ChineseTraditional,
    EndOfList,
};

struct LanguageData{
    std::string code;
    std::string name;
    double random_match_chance;
};

class LanguageSet{
public:
    LanguageSet() = default;
    LanguageSet(std::initializer_list<Language> list)
        : m_set(list)
    {}

    bool operator[](Language language) const;
    void operator+=(Language language);
    void operator-=(Language language);
    void operator+=(const LanguageSet& set);
    void operator-=(const LanguageSet& set);

    std::set<Language>::const_iterator begin() const{
        return m_set.begin();
    }
    std::set<Language>::const_iterator end() const{
        return m_set.end();
    }


private:
    std::set<Language> m_set;
};


const LanguageData& language_data(Language language);
Language language_code_to_enum(const std::string& language);
std::string language_warning(Language language);




}
#endif
