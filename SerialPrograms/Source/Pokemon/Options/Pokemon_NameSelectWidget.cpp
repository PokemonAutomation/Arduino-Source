/*  Pokemon Name Select Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QCompleter>
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Logging/Logger.h"
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
    const SpriteDatabase& icons,
    const std::vector<std::string>& slugs,
    const std::string& current_slug,
    const std::map<std::string, std::string>* display_names,
    const std::map<std::string, std::string>* display_name_to_slug,
    const std::map<std::string, std::pair<std::string, QIcon>>* extra_names,
    const std::vector<std::string>* extra_name_list,
    const std::map<std::string, std::string>* extra_display_name_to_slug
)
    : NoWheelComboBox(&parent)
    , m_display_name_to_slug(display_name_to_slug)
    , m_extra_display_name_to_slug(extra_display_name_to_slug)
{
    this->setEditable(true);
    this->setInsertPolicy(QComboBox::NoInsert);
    this->completer()->setCompletionMode(QCompleter::PopupCompletion);
    this->completer()->setFilterMode(Qt::MatchContains);
    this->setIconSize(QSize(25, 25));

//    WallClock time0 = current_time();

    //  A more optimized version.
    QStringList list;
    if (display_names == nullptr){
        for (const std::string& slug : slugs){
            list.append(QString::fromStdString(get_pokemon_name(slug).display_name()));
        }
    }else{
        for (const std::string& slug : slugs){
            auto it = display_names->find(slug);
            if (it == display_names->end()){
                throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "In display_names slug not found: " + slug);
            }
            list.append(QString::fromStdString(it->second));
        }
    }
    if (extra_names && extra_name_list){
        for(const std::string& slug : *extra_name_list){
            const auto it = extra_names->find(slug);
            if (it == extra_names->end()){
                throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Extra slug not found: " + slug);
            }
            list.append(QString::fromStdString(it->second.first));
        }
    }
    this->addItems(list);

    // Initialize the widget to at least select sth.
    if (slugs.size() > 0){
        this->setCurrentIndex(0);
    }

    // Set the widget to select the pokemon `current_slug`.
    for (size_t index = 0; index < slugs.size(); index++){
        const std::string& slug = slugs[index];

        const SpriteDatabase::Sprite* sprite = icons.get_nothrow(slug);
        if (sprite == nullptr){
            global_logger_tagged().log("Missing sprite for: " + slug, COLOR_RED);
        }else{
            QPixmap pixmap = QPixmap::fromImage(sprite->icon.to_QImage_ref());
            this->setItemIcon((int)index, pixmap);
        }

        if (slug == current_slug){
            this->setCurrentIndex((int)index);
        }
    }
    if (extra_names && extra_name_list){
        for (size_t index = 0; index < extra_name_list->size(); index++){
            const std::string& slug = extra_name_list->at(index);
            const auto it = extra_names->find(slug);
            if (it == extra_names->end()){
                global_logger_tagged().log("Missing sprite for extra slug: " + slug, COLOR_RED);
            }else{
                this->setItemIcon((int)(index + slugs.size()), it->second.second);
            }

            if (slug == current_slug){
                this->setCurrentIndex((int)(index + slugs.size()));
            }
        }
    }

//    WallClock time3 = current_time();
//    cout << std::chrono::duration_cast<std::chrono::milliseconds>(time3 - time0).count() / 1000. << endl;

    update_size_cache();
}




std::string NameSelectWidget::slug() const{
    std::string current_text = currentText().toStdString();
    if (m_extra_display_name_to_slug){
        auto it = m_extra_display_name_to_slug->find(current_text);
        if (it != m_extra_display_name_to_slug->end()){
            return it->second;
        }
    }
    if (m_display_name_to_slug){
        auto it = m_display_name_to_slug->find(current_text);
        if (it != m_display_name_to_slug->end()){
            return it->second;
        }
        return PokemonNames::NULL_SLUG;
    }
    return parse_pokemon_name_nothrow(current_text);
}





}
}
