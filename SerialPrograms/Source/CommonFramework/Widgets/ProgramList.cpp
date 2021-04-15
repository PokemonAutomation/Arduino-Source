/*  UI List for all the Programs
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Qt/StringException.h"
#include "CommonFramework/Panels/RightPanel.h"
#include "CommonFramework/Windows/MainWindow.h"
#include "PanelList.h"
#include "ProgramList.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


ProgramListUI::ProgramListUI(MainWindow& parent)
    : m_parent(parent)
    , m_text_width(0)
    , m_current(nullptr)
    , m_active_panel(nullptr)
{
    QFontMetrics fm(this->font());
    for (const auto& item : PROGRAM_LIST()){
        addItem(item->name());
        QListWidgetItem* list_item = this->item(this->count() - 1);
        list_item->setForeground(item->color());
        m_text_width = std::max(m_text_width, fm.width(item->name()));
//        cout << m_text_width << " / " << list_item->sizeHint().width() << endl;
    }
//    setMaximumWidth(m_text_width);

    connect(this, &QListWidget::itemClicked, this, &ProgramListUI::row_selected);
//    connect(this, &QListWidget::currentRowChanged, this, &ProgramListUI::row_changed);
}

void ProgramListUI::row_selected(QListWidgetItem* item){
//    if (m_current == item){
//        return;
//    }

    auto iter = PROGRAM_MAP().find(item->text());
    if (iter == PROGRAM_MAP().end()){
//        std::cout << item->text().toUtf8().data() << std::endl;
        throw StringException("Invalid program name: " + item->text());
    }
    m_parent.change_panel(*iter->second);
    m_current = item;
}


}
