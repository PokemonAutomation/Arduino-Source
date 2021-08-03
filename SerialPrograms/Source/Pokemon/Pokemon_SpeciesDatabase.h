/*  Pokemon Species Database
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Pokemon_SpeciesDatabase_H
#define PokemonAutomation_Pokemon_SpeciesDatabase_H

#include <vector>
#include <map>
#include <QString>

#include "CommonFramework/Language.h"

namespace PokemonAutomation{
namespace Pokemon{


class SpeciesData{
public:
    const std::string& slug() const{ return m_slug; };
    const QString& display_name() const{ return m_display_name; }
    const QString& display_name(Language language) const;

private:
    friend struct SpeciesSlugDatabase;

    std::string m_slug;
    QString m_display_name;
    std::map<Language, QString> m_display_names;
};

const std::vector<std::string>& NATIONAL_DEX_SLUGS();
const std::map<std::string, SpeciesData>& ALL_POKEMON();

const SpeciesData& species_slug_to_data(const std::string& slug);
const std::string& species_display_name_to_slug(const QString& display_name);
const std::string& species_display_name_to_slug_nothrow(const QString& display_name);


}
}
#endif
