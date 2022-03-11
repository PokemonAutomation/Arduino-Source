/*  Panel List
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QJsonObject>
#include <QMessageBox>
#include "Common/Cpp/Exceptions.h"
#include "Common/Qt/QtJsonTools.h"
#include "CommonFramework/PersistentSettings.h"
#include "PanelList.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{

const QString PanelList::JSON_PROGRAM_PANEL = "ProgramPanel";

PanelList::PanelList(QTabWidget& parent, QString label, PanelListener& listener)
    : QListWidget(&parent)
    , m_label(label)
    , m_listener(listener)
{}
void PanelList::add_divider(QString label){
    m_panels.emplace_back(std::move(label), nullptr);
}
void PanelList::finish_panel_setup(){
//    QFontMetrics fm(this->font());
    for (const auto& item : m_panels){
        //  Label/divider
        if (item.second == nullptr){
            addItem(item.first);
            QListWidgetItem* list_item = this->item(this->count() - 1);
            QFont font = list_item->font();
            font.setBold(true);
            list_item->setFont(font);
//            list_item->setTextAlignment(Qt::AlignCenter);
            continue;
        }

        //  Program
        const QString& display_name = item.second->display_name();
        if (!m_panel_map.emplace(display_name, item.second.get()).second){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Duplicate program name: " + display_name.toStdString());
        }

        addItem(display_name);
        QListWidgetItem* list_item = this->item(this->count() - 1);
        Color color = item.second->color();
        if (color){
            QColor qcolor = QColor((uint32_t)color);
            list_item->setForeground(qcolor);
        }
        list_item->setToolTip(item.second->description());
    }
    connect(
        this, &QListWidget::itemClicked,
        this, [=](QListWidgetItem* item){
            handle_panel_clicked(item->text());
        }
    );
}

void PanelList::handle_panel_clicked(const QString text){
    auto iter = m_panel_map.find(text);
    if (iter == m_panel_map.end()){
        PERSISTENT_SETTINGS().panels.insert(JSON_PROGRAM_PANEL, "");
        return;
    }
    const PanelDescriptor* descriptor = iter->second;
    try{
        std::unique_ptr<PanelInstance> panel = descriptor->make_panel();
        const QString identifier = QString::fromStdString(descriptor->identifier());
        panel->from_json(PERSISTENT_SETTINGS().panels[identifier]);
        m_listener.on_panel_construct(std::move(panel));

        PERSISTENT_SETTINGS().panels.insert(JSON_PROGRAM_PANEL, iter->first);
    }catch (const Exception& error){
        QMessageBox box;
        box.critical(
            nullptr,
            "Error",
            "Failed to load program.\n\n" + QString::fromStdString(error.to_str())
        );
    }
}

void PanelList::set_panel(const QString& panel_name){
    const auto panels = findItems(panel_name, Qt::MatchExactly);
    if (panels.size() > 0){
        setCurrentItem(panels[0]);
    }

    handle_panel_clicked(panel_name);
}



}


