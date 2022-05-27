/*  Pokemon Name List Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Pokemon_PokemonNameListOption_H
#define PokemonAutomation_Pokemon_PokemonNameListOption_H

#include <map>
#include <vector>
#include "CommonFramework/Options/EditableTableOption.h"

class QIcon;

namespace PokemonAutomation{
namespace Pokemon{

struct ExtraNames{
    // mapping from the slug of the extra names to their display names and icons
    std::map<std::string, std::pair<QString, QIcon>> names;
    // list of extra name slugs to display on the widget, after all the pokemon names listed on the widget
    std::vector<std::string> name_list;
    // mapping from the display name of the extra names to their slugs.
    std::map<QString, std::string> display_name_to_slug;
};

class PokemonNameListFactory : public EditableTableFactory{
public:
    PokemonNameListFactory(const std::map<std::string, QIcon>& icons);
    PokemonNameListFactory(const std::map<std::string, QIcon>& icons, std::vector<std::string> slug_list,
        const ExtraNames* extra_names = nullptr);

    virtual QStringList make_header() const override;
    virtual std::unique_ptr<EditableTableRow> make_row() const override;

private:
    friend class PokemonNameListRow;
    const std::map<std::string, QIcon>& m_icons;
    std::vector<std::string> m_slug_list;
    const ExtraNames* m_extra_names = nullptr;
};


class PokemonNameList : public PokemonNameListFactory, public EditableTableOption{
public:
    // Show all pokemon and 
    PokemonNameList(QString label, const std::map<std::string, QIcon>& icons);
    PokemonNameList(QString label, const std::map<std::string, QIcon>& icons, std::vector<std::string> slug_list,
        const ExtraNames* extra_names = nullptr);

    const std::string& operator[](size_t index) const;
};


}
}
#endif
