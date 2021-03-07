/*  UI List for all the Settings
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Qt/StringException.h"
#include "CommonFramework/Panels/RightPanel.h"
#include "CommonFramework/Windows/MainWindow.h"
#include "PanelList.h"
#include "SettingList.h"

namespace PokemonAutomation{


SettingListUI::SettingListUI(MainWindow& parent)
    : m_parent(parent)
    , m_text_width(0)
    , m_current(nullptr)
    , m_active_panel(nullptr)
{
    connect(this, &QListWidget::itemClicked, this, &SettingListUI::row_selected);
//    connect(this, &QListWidget::currentRowChanged, this, &SettingListUI::row_changed);

    const auto& list = SETTINGS_LIST();
    if (list.empty()){
        setMaximumHeight(50);
        return;
    }

    QFontMetrics fm(this->font());
    for (const auto& item : SETTINGS_LIST()){
        addItem(item->name());
        this->item(this->count() - 1)->setForeground(item->color());
        m_text_width = std::max(m_text_width, fm.width(item->name()));
//        cout << m_text_width << endl;
    }
//    setMaximumWidth(m_width);

    setMaximumHeight(4 + list.size() * (sizeHintForRow(0) + 2));
}

void SettingListUI::row_selected(QListWidgetItem* item){
//    if (m_current == item){
//        return;
//    }

    auto iter = SETTINGS_MAP().find(item->text());
    if (iter == SETTINGS_MAP().end()){
//        std::cout << item->text().toUtf8().data() << std::endl;
        throw StringException("Invalid program name: " + item->text());
    }
    m_parent.change_panel(*iter->second);
    m_current = item;
}



}
