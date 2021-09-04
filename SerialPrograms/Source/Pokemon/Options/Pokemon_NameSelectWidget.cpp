/*  Pokemon Name Select Widget
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QCompleter>
#include "Common/Cpp/Exception.h"
#include "CommonFramework/Tools/Logger.h"
#include "Pokemon/Resources/Pokemon_PokemonNames.h"
#include "PokemonSwSh/Resources/PokemonSwSh_PokemonSprites.h"
#include "Pokemon_NameSelectWidget.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace Pokemon{

NameSelectWidget::NameSelectWidget(
    QWidget& parent,
    const std::vector<std::string>& slugs,
    const std::string& current_slug
)
    : NoWheelComboBox(&parent)
{
    this->setEditable(true);
    this->setInsertPolicy(QComboBox::NoInsert);
    this->completer()->setCompletionMode(QCompleter::PopupCompletion);
    this->completer()->setFilterMode(Qt::MatchContains);

    for (size_t index = 0; index < slugs.size(); index++){
        using namespace NintendoSwitch::PokemonSwSh;

        const std::string& slug = slugs[index];
        const PokemonSprite* sprites = get_pokemon_sprite_nothrow(slug);
        if (sprites == nullptr){
            this->addItem(
                get_pokemon_name(slug).display_name()
            );
            global_logger().log("Missing sprite for: " + slug, "red");
        }else{
            this->addItem(
                sprites->icon(),
                get_pokemon_name(slug).display_name()
            );
        }
        if (slug == current_slug){
            this->setCurrentIndex((int)index);
        }
    }
}
std::string NameSelectWidget::slug() const{
    return parse_pokemon_name_nothrow(currentText());
}



}
}
