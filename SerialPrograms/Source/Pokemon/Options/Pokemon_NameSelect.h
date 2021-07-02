/*  Pokemon Name Select
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Pokemon_PokemonNameSelect_H
#define PokemonAutomation_Pokemon_PokemonNameSelect_H

#include "CommonFramework/Options/StringSelect.h"

namespace PokemonAutomation{
namespace Pokemon{


struct PokemonNameSelectData{
    PokemonNameSelectData(const QString& json_file);
    const std::vector<QString>& cases() const{ return m_list; }

protected:
    std::vector<QString> m_list;
    std::map<QString, std::string> m_display_to_token;
};


class PokemonNameSelect : public PokemonNameSelectData, public StringSelect{
public:
    PokemonNameSelect(
        QString label,
        const QString& json_file
    );

    const std::string& token() const;

private:
};


}
}
#endif
