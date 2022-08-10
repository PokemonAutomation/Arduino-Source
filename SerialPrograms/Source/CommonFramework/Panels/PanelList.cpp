/*  Panel List
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QMessageBox>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Json/JsonTools.h"
#include "CommonFramework/PersistentSettings.h"
#include "PanelList.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{

const std::string PanelList::JSON_PROGRAM_PANEL = "ProgramPanel";

PanelList::PanelList(QTabWidget& parent, std::string label, PanelHolder& holder)
    : QListWidget(&parent)
    , m_label(label)
    , m_panel_holder(holder)
{}
void PanelList::add_divider(std::string label){
    m_panels.emplace_back(std::move(label), nullptr);
}
void PanelList::finish_panel_setup(){
//    QFontMetrics fm(this->font());
    for (const auto& item : m_panels){
        //  Label/divider
        if (item.second == nullptr){
            addItem(QString::fromStdString(item.first));
            QListWidgetItem* list_item = this->item(this->count() - 1);
            QFont font = list_item->font();
            font.setBold(true);
            list_item->setFont(font);
//            list_item->setTextAlignment(Qt::AlignCenter);
            continue;
        }

        //  Program
        const std::string& display_name = item.second->display_name();
        if (!m_panel_map.emplace(display_name, item.second.get()).second){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Duplicate program name: " + display_name);
        }

        addItem(QString::fromStdString(display_name));
//        addItem(QString::fromStdString("DM Elvis for FREE SHINIES!!!"));
        QListWidgetItem* list_item = this->item(this->count() - 1);
        Color color = item.second->color();
        if (color){
            QColor qcolor = QColor((uint32_t)color);
            list_item->setForeground(qcolor);
        }
        list_item->setToolTip(QString::fromStdString(item.second->description()));
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
    const PanelDescriptor* descriptor = iter->second;
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


