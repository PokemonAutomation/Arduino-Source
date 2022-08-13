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
#include "PokemonBDSP/PokemonBDSP_Panels.h"
#include "PokemonLA/PokemonLA_Panels.h"
#include "PokemonSV/PokemonSV_Panels.h"
#include "PanelLists.h"

#include <iostream>
using std::cout;
using std::endl;


namespace PokemonAutomation{

const std::string JSON_TAB = "ProgramTab";
const std::array<std::string, 4> JSON_TAB_NAMES = {
    "Switch",
    "SwSh",
    "BDSP",
    "LA"
};


ProgramTabs::ProgramTabs(QWidget& parent, PanelHolder& holder)
    : QTabWidget(&parent)
{
    add(new PanelList(*this, holder,
        "Switch",  "Nintendo Switch",
        NintendoSwitch::make_panels()
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

    connect(this, &ProgramTabs::currentChanged, this, [&](int index){
        if (index >= 0 && (size_t)index < JSON_TAB_NAMES.size()){
            PERSISTENT_SETTINGS().panels[JSON_TAB] = JSON_TAB_NAMES[index];
        }
    });
}

void ProgramTabs::load_persistent_panel(){
    const std::string* str = PERSISTENT_SETTINGS().panels.get_string(JSON_TAB);
    if (str == nullptr){
        return;
    }
    for(size_t i = 0; i < JSON_TAB_NAMES.size(); i++){
        if (*str == JSON_TAB_NAMES[i]){
            setCurrentIndex((int)i);
            break;
        }
    }
    str = PERSISTENT_SETTINGS().panels.get_string(PanelList::JSON_PROGRAM_PANEL);
    if (str != nullptr){
        m_lists[currentIndex()]->set_panel(*str);
    }
}

void ProgramTabs::add(PanelList* list){
    addTab(list, QString::fromStdString(list->label()));
    setTabToolTip(count() - 1, QString::fromStdString(list->description()));
    if (list->items() == 0){
        setTabEnabled((int)m_lists.size(), false);
    }
    m_lists.emplace_back(list);
}

QSize ProgramTabs::sizeHint() const{
    QSize size = QTabWidget::sizeHint();
//    cout << size.width() << " x " << size.height() << endl;
//    cout << this->size().width() << " x " << this->size().height() << endl;
    size.setWidth(scale_dpi_width(size.width() + 10));
    return size;
}






}
