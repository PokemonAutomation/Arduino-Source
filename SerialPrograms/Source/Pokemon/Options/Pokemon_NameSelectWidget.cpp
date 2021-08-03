/*  Pokemon Name Select Widget
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QCompleter>
#include "Common/Cpp/Exception.h"
#include "Pokemon/Pokemon_SpeciesDatabase.h"
#include "Pokemon_NameSelectWidget.h"

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

    const std::map<std::string, SpeciesData>& database = ALL_POKEMON();
    for (size_t index = 0; index < slugs.size(); index++){
        const std::string& slug = slugs[index];
        auto iter = database.find(slug);
        if (iter == database.end()){
            PA_THROW_StringException("Invalid slug: " + slug);
        }
        this->addItem(iter->second.display_name());
        if (slug == current_slug){
            this->setCurrentIndex((int)index);
        }
    }
}
std::string NameSelectWidget::slug() const{
    return species_display_name_to_slug_nothrow(currentText());
}



}
}
