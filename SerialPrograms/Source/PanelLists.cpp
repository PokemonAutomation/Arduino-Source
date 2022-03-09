/*  Left-Side Panel
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QLabel>
#include "Common/Qt/QtJsonTools.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/PersistentSettings.h"
#include "NintendoSwitch/NintendoSwitch_Panels.h"
#include "PokemonSwSh/PokemonSwSh_Panels.h"
#include "PokemonBDSP/Panels_PokemonBDSP.h"
#include "PokemonLA/Panels_PokemonLA.h"
#include "PanelLists.h"

#include <iostream>
using std::cout;
using std::endl;


namespace PokemonAutomation{

const QString JSON_TAB = "ProgramTab";
const std::array<QString, 4> JSON_TAB_NAMES = {
    "Switch",
    "SwSh",
    "BDSP",
    "LA"
};


ProgramTabs::ProgramTabs(QWidget& parent, PanelListener& listener)
    : QTabWidget(&parent)
{
    add(new NintendoSwitch::Panels(*this, listener));
    add(new NintendoSwitch::PokemonSwSh::Panels(*this, listener));
    add(new NintendoSwitch::PokemonBDSP::Panels(*this, listener));
    add(new NintendoSwitch::PokemonLA::Panels(*this, listener));

    connect(this, &ProgramTabs::currentChanged, this, [&](int index){
        if (index >= 0 && (size_t)index < JSON_TAB_NAMES.size()){
            PERSISTENT_SETTINGS().panels.insert(JSON_TAB, JSON_TAB_NAMES[index]);
        }
    });
}

void ProgramTabs::load_persistent_panel(){
    QString str;
    if (json_get_string(str, PERSISTENT_SETTINGS().panels, JSON_TAB)){
        for(size_t i = 0; i < JSON_TAB_NAMES.size(); i++){
            if (str == JSON_TAB_NAMES[i]){
                setCurrentIndex(i);
                break;
            }
        }
        if (json_get_string(str, PERSISTENT_SETTINGS().panels, PanelList::JSON_PROGRAM_PANEL)){
            m_lists[currentIndex()]->set_panel(str);
        }
    }
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
