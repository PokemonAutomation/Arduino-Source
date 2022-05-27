/*  Pokemon Name Select Widget
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Pokemon_PokemonNameSelectWidget_H
#define PokemonAutomation_Pokemon_PokemonNameSelectWidget_H

#include "Common/Qt/NoWheelComboBox.h"

class QString;


namespace PokemonAutomation{
namespace Pokemon{

// A widget to select a pokemon
class NameSelectWidget : public NoWheelComboBox{
public:
    // icons: pokemon slug -> icon. This map can be larger than the list of pokemon displayed on the widget.
    // slugs: a list of pokemon slugs to choose from on the widget.
    // current_slug: current selected pokemon
    // extra_names: in rare cases we may add names that are not pokemon into the widget (like MMOs in LA).
    //   `extra_names` gives the mapping from the slug of the extra names to their display names and icons.
    // extra_name_list: list of extra name slugs to display on the widget, after the pokemon from `slugs`.
    // extra_display_name_to_slug: the mapping from the display name of the extra names to their slugs.
    NameSelectWidget(
        QWidget& parent,
        const std::map<std::string, QIcon>& icons,
        const std::vector<std::string>& slugs,
        const std::string& current_slug,
        const std::map<std::string, std::pair<QString, QIcon>>* extra_names = nullptr,
        const std::vector<std::string>* extra_name_list = nullptr,
        const std::map<QString, std::string>* extra_display_name_to_slug = nullptr
    );

    std::string slug() const;

private:
    const std::map<QString, std::string>* m_extra_display_name_to_slug = nullptr;
};



}
}
#endif
