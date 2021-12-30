/*  Pokemon Ball Select Widget
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QCompleter>
#include "Common/Cpp/Exception.h"
#include "CommonFramework/Logging/Logger.h"
#include "Pokemon/Resources/Pokemon_PokeballNames.h"
#include "PokemonSwSh/Resources/PokemonSwSh_PokeballSprites.h"
#include "Pokemon_BallSelectWidget.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace Pokemon{

BallSelectWidget::BallSelectWidget(
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
        const PokeballSprite* sprites = get_pokeball_sprite_nothrow(slug);
        if (sprites == nullptr){
            this->addItem(
                get_pokeball_name(slug).display_name()
            );
            global_logger_tagged().log("Missing sprite for: " + slug, COLOR_RED);
        }else{
            this->addItem(
                sprites->icon(),
                get_pokeball_name(slug).display_name()
            );
        }
        if (slug == current_slug){
            this->setCurrentIndex((int)index);
        }
    }
}
std::string BallSelectWidget::slug() const{
    return parse_pokeball_name_nothrow(currentText());
}



}
}
