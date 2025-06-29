/*  Daily Highlight Database
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_DailyHighlightDatabase_H
#define PokemonAutomation_PokemonSwSh_DailyHighlightDatabase_H

#include <map>
#include "CommonTools/Options/StringSelectOption.h"

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonSwSh {


class DailyHighlightDatabase {
public:
    DailyHighlightDatabase(const char* resource_path);

    std::pair<uint16_t, uint16_t> get_range_for_slug(const std::string& slug) const;
    const StringSelectDatabase& database() const {
        return m_stringselect_database;
    }

private:
    std::map<std::string, std::pair<uint16_t, uint16_t>> m_slug_to_range;
    StringSelectDatabase m_stringselect_database;
};




}
}
}
#endif
