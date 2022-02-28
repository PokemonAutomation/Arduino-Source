/*  Pokemon Berry Select Widget
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QCompleter>
#include "CommonFramework/Logging/LoggerQt.h"
#include "Pokemon/Resources/Pokemon_BerryNames.h"
#include "Pokemon/Resources/Pokemon_BerrySprites.h"
#include "Pokemon_BerrySelectWidget.h"

namespace PokemonAutomation{
namespace Pokemon{

BerrySelectWidget::BerrySelectWidget(
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
    this->setSizeAdjustPolicy(QComboBox::SizeAdjustPolicy::AdjustToContents);

    for (size_t index = 0; index < slugs.size(); index++){
        const std::string& slug = slugs[index];

        const BerrySprite* sprite = get_berry_sprite_nothrow(slug);
        if (sprite == nullptr){
            this->addItem(
                get_berry_name(slug).display_name()
            );
            global_logger_tagged().log("Missing sprite for: " + slug, COLOR_RED);
        }else{
            this->addItem(
                sprite->icon(),
                get_berry_name(slug).display_name()
            );
        }

        if (slug == current_slug){
            this->setCurrentIndex((int)index);
        }
    }
}
std::string BerrySelectWidget::slug() const{
    return parse_berry_name_nothrow(currentText());
}



}
}
