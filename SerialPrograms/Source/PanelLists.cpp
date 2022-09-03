/*  Left-Side Panel
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <array>
#include <QLabel>
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/PersistentSettings.h"
#include "CommonFramework/Windows/DpiScaler.h"
#include "NintendoSwitch/NintendoSwitch_Panels.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/PokemonSwSh_Panels.h"
#include "PokemonHome/PokemonHome_Panels.h"
#include "PokemonBDSP/PokemonBDSP_Panels.h"
#include "PokemonLA/PokemonLA_Panels.h"
#include "PokemonSV/PokemonSV_Panels.h"
#include "PanelLists.h"

#include <iostream>
using std::cout;
using std::endl;


namespace PokemonAutomation{


class MyTabBar : public QTabBar{
public:
    using QTabBar::QTabBar;
#if 0
    virtual QSize tabSizeHint(int index) const{
        QSize ret = QTabBar::tabSizeHint(index);
        cout << ret.width() << " x " << ret.height() << endl;
        return QSize(ret.width() * 0.8, ret.height());
    }
#endif
};



ProgramTabs::ProgramTabs(QWidget& parent, PanelHolder& holder)
    : QTabWidget(&parent)
{
    setTabBar(new MyTabBar(this));

    add(new PanelList(*this, holder,
        "Switch",  "Nintendo Switch",
        NintendoSwitch::make_panels()
    ));
    add(new PanelList(*this, holder,
        "Home", Pokemon::STRING_POKEMON + " Home",
        NintendoSwitch::PokemonHome::make_panels()
    ));
    add(new PanelList(*this, holder,
        "SwSh", Pokemon::STRING_POKEMON + " Sword and Shield",
        NintendoSwitch::PokemonSwSh::make_panels()
    ));
    add(new PanelList(*this, holder,
        "BDSP", Pokemon::STRING_POKEMON + " Brilliant Diamond and Shining Pearl",
        NintendoSwitch::PokemonBDSP::make_panels()
    ));
    add(new PanelList(*this, holder,
        "LA", Pokemon::STRING_POKEMON + " Legends Arceus",
        NintendoSwitch::PokemonLA::make_panels()
    ));
    add(new PanelList(*this, holder,
        "SV", Pokemon::STRING_POKEMON + " Scarlet and Violet",
        NintendoSwitch::PokemonSV::make_panels()
    ));

#if 0
    QTabBar* bar = this->tabBar();
    for (int c = 0; c < bar->count(); c++){
//        cout << "asdf" << endl;
        const char* name = bar->tabData(c).typeName();
        if (name){
            cout << name << endl;
        }else{
            cout << "(null)" << endl;
        }
//        bar->tabButton(c, QTabBar::RightSide)->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    }
#endif

    connect(this, &ProgramTabs::currentChanged, this, [&](int index){
        if (index >= 0 && (size_t)index < m_lists.size()){
            PERSISTENT_SETTINGS().panels["ProgramTab"] = m_lists[index]->label();
        }
    });
}

void ProgramTabs::load_persistent_panel(){
    const std::string* str = PERSISTENT_SETTINGS().panels.get_string("ProgramTab");
    if (str == nullptr){
        return;
    }
    auto iter = m_tab_map.find(*str);
    if (iter == m_tab_map.end()){
        return;
    }
    setCurrentIndex(iter->second);
    str = PERSISTENT_SETTINGS().panels.get_string(PanelList::JSON_PROGRAM_PANEL);
    if (str != nullptr){
        m_lists[currentIndex()]->set_panel(*str);
    }
}

void ProgramTabs::add(PanelList* list){
    int index = count();
    addTab(list, QString::fromStdString(list->label()));
    setTabToolTip(index, QString::fromStdString(list->description()));
    if (list->items() == 0){
        setTabEnabled((int)m_lists.size(), false);
    }
    m_lists.emplace_back(list);
    if (!m_tab_map.emplace(list->label(), index).second){
        m_lists.pop_back();
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Duplicate Tab Name: " + list->label());
    }
}

QSize ProgramTabs::sizeHint() const{
    QSize size = QTabWidget::sizeHint();
//    cout << size.width() << " x " << size.height() << endl;
//    cout << this->size().width() << " x " << this->size().height() << endl;
    size.setWidth(scale_dpi_width(size.width() + 10));
    return size;
}






}
