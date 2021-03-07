/*  UI List for all the Programs
 *
 *  From: https://github.com/Mysticial/Pokemon-Automation-SwSh-Arduino-Scripts
 *
 */

#include <vector>
#include "Common/Qt/StringException.h"
#include "Common/Qt/QtJsonTools.h"
#include "UI/MainWindow.h"
#include "Panels/PanelList.h"
#include "ProgramListUI.h"

//#include <iostream>
//using namespace std;

namespace PokemonAutomation{


ProgramListUI::ProgramListUI(MainWindow& parent)
    : m_parent(parent)
    , m_text_width(0)
{
//    setSizeAdjustPolicy(SizeAdjustPolicy::AdjustToContents);
//    setMinimumWidth();
//    setMaximumWidth(300);
//    sizeHintForRow(100);
//    sizeHintForColumn(100);

    connect(this, &QListWidget::itemClicked, this, &ProgramListUI::row_selected);
    connect(this, &QListWidget::currentRowChanged, this, &ProgramListUI::row_changed);

    QFontMetrics fm(this->font());
    for (const auto& item : PROGRAM_LIST()){
        addItem(item->name());
        m_text_width = std::max(m_text_width, fm.width(item->name()));
//        cout << m_text_width << endl;
    }
//    setMaximumWidth(m_width);
}

void ProgramListUI::row_selected(QListWidgetItem* item){
    auto iter = PROGRAM_MAP().find(item->text());
    if (iter == PROGRAM_MAP().end()){
//        std::cout << item->text().toUtf8().data() << std::endl;
        throw StringException("Invalid program name: " + item->text());
    }

    m_parent.change_panel(*iter->second);
}
void ProgramListUI::row_changed(int row){
    row_selected(this->item(row));
}



}
