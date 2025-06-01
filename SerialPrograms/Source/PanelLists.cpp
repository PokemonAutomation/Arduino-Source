/*  Left-Side Panel
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QStandardItemModel>
#include <QVBoxLayout>
#include <QLabel>
#include "Common/Qt/NoWheelComboBox.h"
#include "CommonFramework/PersistentSettings.h"
#include "CommonFramework/Windows/DpiScaler.h"
#include "CommonFramework/Panels/UI/PanelListWidget.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "ML/ML_Panels.h"
#include "NintendoSwitch/NintendoSwitch_Panels.h"
#include "PokemonSwSh/PokemonSwSh_Panels.h"
#include "PokemonHome/PokemonHome_Panels.h"
#include "PokemonBDSP/PokemonBDSP_Panels.h"
#include "PokemonLA/PokemonLA_Panels.h"
#include "PokemonLGPE/PokemonLGPE_Panels.h"
#include "PokemonRSE/PokemonRSE_Panels.h"
#include "PokemonSV/PokemonSV_Panels.h"
#include "ZeldaTotK/ZeldaTotK_Panels.h"
#include "PanelLists.h"

//#include <iostream>
//using std::cout;
//using std::endl;


namespace PokemonAutomation{



ProgramSelect::ProgramSelect(QWidget& parent, PanelHolder& holder)
    : QGroupBox("Program Select", &parent)
    , m_holder(holder)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignTop);
    m_dropdown = new NoWheelComboBox(this);
    layout->addWidget(m_dropdown);



    add(std::make_unique<NintendoSwitch::PanelListFactory>());
    add(std::make_unique<NintendoSwitch::PokemonHome::PanelListFactory>());
    add(std::make_unique<NintendoSwitch::PokemonLGPE::PanelListFactory>());
    add(std::make_unique<NintendoSwitch::PokemonSwSh::PanelListFactory>());
    add(std::make_unique<NintendoSwitch::PokemonBDSP::PanelListFactory>());
    add(std::make_unique<NintendoSwitch::PokemonLA::PanelListFactory>());
    add(std::make_unique<NintendoSwitch::PokemonSV::PanelListFactory>());
    if (PreloadSettings::instance().DEVELOPER_MODE){
        add(std::make_unique<NintendoSwitch::PokemonRSE::PanelListFactory>());
    }
    add(std::make_unique<NintendoSwitch::ZeldaTotK::PanelListFactory>());
    if (PreloadSettings::instance().DEVELOPER_MODE){
        add(std::make_unique<ML::PanelListFactory>());
    }


    //  Load the 1st list by default.
    m_dropdown->setCurrentIndex(0);
    m_active_index = 0;
    m_active_list = m_lists[0]->make_QWidget(*this, m_holder);
    layout->addWidget(m_active_list);

    connect(
        m_dropdown, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated),
        this, [this](int index){
            change_list(index);
        }
    );
}

void ProgramSelect::add(std::unique_ptr<PanelListDescriptor> list){
    int index = m_dropdown->count();
    m_dropdown->addItem(QString::fromStdString(list->name()));
    m_lists.emplace_back(std::move(list));
    const PanelListDescriptor& back = *m_lists.back();
    if (!m_tab_map.emplace(back.name(), index).second){
        m_lists.pop_back();
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Duplicate Category Name: " + back.name());
    }
    if (!back.enabled()){
        auto* model = qobject_cast<QStandardItemModel*>(m_dropdown->model());
        if (model != nullptr){
            QStandardItem* line_handle = model->item(index);
            if (line_handle != nullptr){
                line_handle->setEnabled(false);
            }
        }
    }
}



void ProgramSelect::load_persistent_panel(){
    const std::string* str = PERSISTENT_SETTINGS().panels.get_string("ProgramCategory");
    if (str == nullptr){
        return;
    }
    auto iter = m_tab_map.find(*str);
    if (iter == m_tab_map.end()){
        return;
    }
    m_dropdown->setCurrentIndex(iter->second);
    change_list(iter->second);
    str = PERSISTENT_SETTINGS().panels.get_string(PanelListWidget::JSON_PROGRAM_PANEL);
    if (str != nullptr){
        m_active_list->set_panel(*str);
    }
}

void ProgramSelect::change_list(int index){
    if (m_active_index == index && m_active_list != nullptr){
        return;
    }
    m_dropdown->setCurrentIndex(index);
    m_active_index = index;
    PERSISTENT_SETTINGS().panels["ProgramCategory"] = m_lists[index]->name();
    delete m_active_list;
    m_active_list = m_lists[index]->make_QWidget(*this, m_holder);
    layout()->addWidget(m_active_list);
}

QSize ProgramSelect::sizeHint() const{
    QSize size = QGroupBox::sizeHint();
//    cout << size.width() << " x " << size.height() << endl;
//    cout << this->size().width() << " x " << this->size().height() << endl;
    size.setWidth(scale_dpi_width(size.width() + 10));
    return size;
}







}
