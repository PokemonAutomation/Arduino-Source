/*  Pokemon Name List Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Pokemon_PokemonNameListOption_H
#define PokemonAutomation_Pokemon_PokemonNameListOption_H

#include <map>
#include <vector>
#include "Common/Cpp/Options/EditableTableOption2.h"
#include "CommonFramework/Options/StringSelectOption.h"
#include "CommonFramework/Options/EditableTableOption.h"
#include "CommonFramework/Resources/SpriteDatabase.h"
#include "Pokemon/Pokemon_Strings.h"

namespace PokemonAutomation{
namespace Pokemon{


struct DisplayNameMapping{
    // slug -> display name
    std::map<std::string, std::string> display_names;
    // display name -> slug
    std::map<std::string, std::string> display_name_to_slug;
};


// Pass extra names to the pokemon name list
struct ExtraNames{
    // mapping from the slug of the extra names to their display names and icons
    std::map<std::string, std::pair<std::string, QIcon>> names;
    // list of extra name slugs to display on the widget, after all the pokemon names listed on the widget
    std::vector<std::string> name_list;
    // mapping from the display name of the extra names to their slugs.
    std::map<std::string, std::string> display_name_to_slug;
};

class PokemonNameListFactory : public EditableTableFactory{
public:
    PokemonNameListFactory(
        const SpriteDatabase& icons,
        std::vector<std::string> slug_list,
        const DisplayNameMapping* display_name_mapping = nullptr,
        const ExtraNames* extra_names = nullptr
    );

    virtual std::vector<std::string> make_header() const override;
    virtual std::unique_ptr<EditableTableRow> make_row() const override;

private:
    friend class PokemonNameListRow;
    const SpriteDatabase& m_icons;
    std::vector<std::string> m_slug_list;
    const DisplayNameMapping* m_display_name_mapping = nullptr;
    const ExtraNames* m_extra_names = nullptr;
};


class PokemonNameList : public PokemonNameListFactory, public EditableTableOption{
public:
    // Config Widget that users can select zero to any number of pokemon in a table, where each table row
    // is a dropdown menu to select a pokemon.
    // label: explanatary text of the config widget.
    // icons: pokemon slug -> pokemon image icon. The mapping to add pokemon icons on the UI.
    // slug_list: list of pokemon slug to display on the UI for the user to choose. If a pokemon in this list does not
    //   appear in `icons`, its icon will be missing on the UI.
    // display_name_mapping: optional mapping between slugs and display names. By default, it uses the mapping defined
    //   in Pokemon/Resources/Pokemon_PokemonNames.h, which is the mapping for all species of pokemon on national dex.
    //   Example usage of `display_name_mapping`: to choose different forms of a single species in MMO in LA.
    // extra_names: optional non-pokemon items to choose on the UI. Those extra items will appear after all the available
    //   pokemon on the UI. Example usage of `extra_names`: choose MMO along with other outbreak pokemon in LA.
    PokemonNameList(
        std::string label,
        const SpriteDatabase& icons,
        std::vector<std::string> slug_list,
        const DisplayNameMapping* display_name_mapping = nullptr,
        const ExtraNames* extra_names = nullptr
    );

    const std::string& operator[](size_t index) const;
};






class PokemonNameListRow2 : public EditableTableRow2{
public:
    PokemonNameListRow2(const StringSelectDatabase& database, const std::string& default_pokemon)
        : pokemon(database, default_pokemon)
    {}
    virtual std::unique_ptr<EditableTableRow2> clone() const override{
        std::unique_ptr<PokemonNameListRow2> ret(new PokemonNameListRow2(pokemon.database(), pokemon.default_slug()));
        ret->pokemon.set_by_index(pokemon.index());
        return ret;
    }

public:
    StringSelectCell pokemon;
};


#if 0
class PokemonNameList2 : public EditableTableOption2{
public:
    PokemonNameList2()
        :
    {}

    std::vector<std::unique_ptr<PokemonNameListRow2>> copy_snapshot() const{
        return EditableTableOption2::copy_snapshot<PokemonNameListRow2>();
    }
    virtual std::vector<std::string> make_header() const override{
        return std::vector<std::string>{
            STRING_POKEMON,
        };
    }
    virtual std::unique_ptr<EditableTableRow2> make_row() const override{

    }

public:
    const StringSelectDatabase& m_database;
    const std::string& m_default_pokemon;
};
#endif





















}
}
#endif
