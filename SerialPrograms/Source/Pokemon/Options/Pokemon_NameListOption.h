/*  Pokemon Name List Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Pokemon_PokemonNameListOption_H
#define PokemonAutomation_Pokemon_PokemonNameListOption_H

#include <map>
#include "CommonFramework/Options/EditableTableOption.h"

class QIcon;

namespace PokemonAutomation{
namespace Pokemon{



class PokemonNameListFactory : public EditableTableFactory{
public:
    PokemonNameListFactory(const std::map<std::string, QIcon>& icons);
    PokemonNameListFactory(const std::map<std::string, QIcon>& icons, std::vector<std::string> slug_list);

    virtual QStringList make_header() const override;
    virtual std::unique_ptr<EditableTableRow> make_row() const override;

private:
    friend class PokemonNameListRow;
    const std::map<std::string, QIcon>& m_icons;
    std::vector<std::string> m_slug_list;
};


class PokemonNameList : public PokemonNameListFactory, public EditableTableOption{
public:
    PokemonNameList(QString label, const std::map<std::string, QIcon>& icons);
    PokemonNameList(QString label, const std::map<std::string, QIcon>& icons, std::vector<std::string> slug_list);

    const std::string& operator[](size_t index) const;
};


}
}
#endif
