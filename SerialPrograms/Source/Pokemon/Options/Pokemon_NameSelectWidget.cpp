/*  Pokemon Name Select Widget
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QCompleter>
#include "CommonFramework/Logging/LoggerQt.h"
#include "Pokemon/Resources/Pokemon_PokemonNames.h"
#include "Pokemon_NameSelectWidget.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace Pokemon{

NameSelectWidget::NameSelectWidget(
    QWidget& parent,
    const std::map<std::string, QIcon>& icons,
    const std::vector<std::string>& slugs,
    const std::string& current_slug
)
    : NoWheelComboBox(&parent)
{
    this->setEditable(true);
    this->setInsertPolicy(QComboBox::NoInsert);
    this->completer()->setCompletionMode(QCompleter::PopupCompletion);
    this->completer()->setFilterMode(Qt::MatchContains);
    this->setIconSize(QSize(25, 25));

    for (size_t index = 0; index < slugs.size(); index++){
        const std::string& slug = slugs[index];

        auto iter = icons.find(slug);
        if (iter == icons.end()){
            this->addItem(
                get_pokemon_name(slug).display_name()
            );
            global_logger_tagged().log("Missing sprite for: " + slug, COLOR_RED);
        }else{
            this->addItem(
                iter->second,
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
