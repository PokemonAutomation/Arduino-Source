/*  Pokemon Name Select Widget
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QCompleter>
#include "CommonFramework/Logging/LoggerQt.h"
#include "Pokemon/Resources/Pokemon_PokemonNames.h"
#include "Pokemon_NameSelectWidget.h"

//#include "Common/Cpp/Time.h"
//#include <iostream>
//using std::cout;
//using std::endl;

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

//    WallClock time0 = current_time();

#if 1
    //  A more optimized version.
    QStringList list;
    for (const std::string& slug : slugs){
        list.append(get_pokemon_name(slug).display_name());
    }
    this->addItems(list);

    for (size_t index = 0; index < slugs.size(); index++){
        const std::string& slug = slugs[index];

        auto iter = icons.find(slug);
        if (iter == icons.end()){
            global_logger_tagged().log("Missing sprite for: " + slug, COLOR_RED);
        }else{
            this->setItemIcon((int)index, iter->second);
        }

        if (slug == current_slug){
            this->setCurrentIndex((int)index);
        }
    }
#else
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
#endif

//    WallClock time3 = current_time();
//    cout << std::chrono::duration_cast<std::chrono::milliseconds>(time3 - time0).count() / 1000. << endl;

    update_size_cache();
}
std::string NameSelectWidget::slug() const{
    return parse_pokemon_name_nothrow(currentText());
}





}
}
