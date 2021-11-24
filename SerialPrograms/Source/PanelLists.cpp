/*  Left-Side Panel
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QLabel>
#include "Common/Qt/QtJsonTools.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "NintendoSwitch/NintendoSwitch_Panels.h"
#include "PokemonSwSh/PokemonSwSh_Panels.h"
#include "PokemonBDSP/Panels_PokemonBDSP.h"
#include "PanelLists.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


ProgramTabs::ProgramTabs(QWidget& parent, PanelListener& listener)
    : QTabWidget(&parent)
{
    add(new NintendoSwitch::Panels(*this, listener));
    add(new NintendoSwitch::PokemonSwSh::Panels(*this, listener));
    add(new NintendoSwitch::PokemonBDSP::Panels(*this, listener));
}

void ProgramTabs::add(PanelList* list){
    addTab(list, list->label());
    if (list->items() == 0){
        setTabEnabled((int)m_lists.size(), false);
    }
    m_lists.emplace_back(list);
}

QSize ProgramTabs::sizeHint() const{
    QSize size = QTabWidget::sizeHint();
//    cout << size.width() << " x " << size.height() << endl;
//    cout << this->size().width() << " x " << this->size().height() << endl;
    size.setWidth(size.width() + 10);
    return size;
}






}
