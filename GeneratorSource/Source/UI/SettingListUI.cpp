/*  UI List for all the Settings
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <vector>
#include "Common/Qt/StringException.h"
#include "Common/Qt/QtJsonTools.h"
#include "UI/MainWindow.h"
#include "Panels/PanelList.h"
#include "SettingListUI.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


SettingsListUI::SettingsListUI(MainWindow& parent)
    : m_parent(parent)
    , m_text_width(0)
{
//    setMaximumWidth(300);
//    setSizeAdjustPolicy(QAbstractScrollArea::SizeAdjustPolicy::AdjustToContents);

    connect(this, &QListWidget::itemClicked, this, &SettingsListUI::row_selected);
    connect(this, &QListWidget::currentRowChanged, this, &SettingsListUI::row_changed);

    const auto& list = SETTINGS_LIST();
    if (list.empty()){
        setMaximumHeight(50);
        return;
    }

    QFontMetrics fm(this->font());
//    int width = 0;
    for (const auto& item : list){
        addItem(item->name());
        m_text_width = std::max(m_text_width, fm.width(item->name()));
    }
//    updateGeometry();
//    setMaximumWidth(m_width);

    setMaximumHeight(4 + list.size() * (sizeHintForRow(0) + 2));
}

#if 0
QSize SettingsDialog::sizeHint() const{
    if (model()->rowCount() == 0) return QSize(width(), 0);
    int nToShow = model()->rowCount();
    cout << "asdf" << endl;
    return QSize(width(), nToShow * sizeHintForRow(0));
}
#endif

void SettingsListUI::row_selected(QListWidgetItem* item){
    auto iter = SETTINGS_MAP().find(item->text());
    if (iter == SETTINGS_MAP().end()){
//        std::cout << item->text().toUtf8().data() << std::endl;
        throw StringException("Invalid program name: " + item->text());
    }

    m_parent.change_panel(*iter->second);
}
void SettingsListUI::row_changed(int row){
    row_selected(this->item(row));
}



}



