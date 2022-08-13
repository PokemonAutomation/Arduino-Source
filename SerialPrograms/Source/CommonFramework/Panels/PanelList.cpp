/*  Panel List
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QMessageBox>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Json/JsonTools.h"
#include "CommonFramework/PersistentSettings.h"
#include "PanelInstance.h"
#include "PanelList.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{

const std::string PanelList::JSON_PROGRAM_PANEL = "ProgramPanel";

PanelList::PanelList(
    QTabWidget& parent, PanelHolder& holder,
    std::string label, std::string description,
    std::vector<PanelEntry> list
)
    : QListWidget(&parent)
    , m_label(label)
    , m_description(std::move(description))
    , m_panel_holder(holder)
{
//    QFontMetrics fm(this->font());
    for (PanelEntry& item : list){
        const std::string& display_name = item.display_name;
        PanelDescriptor* descriptor = item.descriptor.get();

        //  Label/divider
        if (descriptor == nullptr){
            addItem(QString::fromStdString(display_name));
            QListWidgetItem* list_item = this->item(this->count() - 1);
            QFont font = list_item->font();
            font.setBold(true);
            list_item->setFont(font);
//            list_item->setTextAlignment(Qt::AlignCenter);
            continue;
        }

        //  Program
        if (!m_panel_map.emplace(display_name, std::move(item.descriptor)).second){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Duplicate program name: " + display_name);
        }

        addItem(QString::fromStdString(display_name));
//        addItem(QString::fromStdString("DM Elvis for FREE SHINIES!!!"));
        QListWidgetItem* list_item = this->item(this->count() - 1);
        Color color = descriptor->color();
        if (color){
            QColor qcolor = QColor((uint32_t)color);
            list_item->setForeground(qcolor);
        }
        list_item->setToolTip(QString::fromStdString(descriptor->description()));
    }
    connect(
        this, &QListWidget::itemClicked,
        this, [=](QListWidgetItem* item){
            handle_panel_clicked(item->text().toStdString());
        }
    );
}


void PanelList::handle_panel_clicked(const std::string& text){
    auto iter = m_panel_map.find(text);
    if (iter == m_panel_map.end()){
        PERSISTENT_SETTINGS().panels[JSON_PROGRAM_PANEL] = "";
        return;
    }
    const PanelDescriptor* descriptor = iter->second.get();
//    cout << descriptor->display_name() << endl;
    if (!m_panel_holder.report_new_panel_intent(*descriptor)){
        return;
    }
    try{
        std::unique_ptr<PanelInstance> panel = descriptor->make_panel();
        panel->from_json(PERSISTENT_SETTINGS().panels[descriptor->identifier()]);
        m_panel_holder.load_panel(std::move(panel));

        PERSISTENT_SETTINGS().panels[JSON_PROGRAM_PANEL] = iter->first;
    }catch (const Exception& error){
        QMessageBox box;
        box.critical(
            nullptr,
            "Error",
            "Failed to load program.\n\n" + QString::fromStdString(error.to_str())
        );
    }
}

void PanelList::set_panel(const std::string& panel_name){
    const auto panels = findItems(QString::fromStdString(panel_name), Qt::MatchExactly);
    if (panels.size() > 0){
        setCurrentItem(panels[0]);
    }

    handle_panel_clicked(panel_name);
}



}


