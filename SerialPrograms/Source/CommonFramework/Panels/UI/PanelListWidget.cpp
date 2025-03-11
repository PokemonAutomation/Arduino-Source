/*  Panel List Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QMessageBox>
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/PersistentSettings.h"
//#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Panels/PanelInstance.h"
#include "PanelListWidget.h"

namespace PokemonAutomation{




PanelListWidget* PanelListDescriptor::make_QWidget(QWidget& parent, PanelHolder& holder) const{
    return new PanelListWidget(
        parent, holder,
        this->make_panels()
    );
}



const std::string PanelListWidget::JSON_PROGRAM_PANEL = "ProgramPanel";

PanelListWidget::PanelListWidget(
    QWidget& parent, PanelHolder& holder,
    std::vector<PanelEntry> list
)
    : QListWidget(&parent)
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
        this, [this](QListWidgetItem* item){
            handle_panel_clicked(item->text().toStdString());
        }
    );
}


void PanelListWidget::handle_panel_clicked(const std::string& text){
    auto iter = m_panel_map.find(text);
    if (iter == m_panel_map.end()){
        PERSISTENT_SETTINGS().panels[JSON_PROGRAM_PANEL] = "";
        return;
    }
    std::shared_ptr<const PanelDescriptor>& descriptor = iter->second;
//    cout << descriptor->display_name() << endl;
    if (!m_panel_holder.report_new_panel_intent(*descriptor)){
        return;
    }
    try{
        std::unique_ptr<PanelInstance> panel = descriptor->make_panel();
//        try{
            panel->from_json(PERSISTENT_SETTINGS().panels[descriptor->identifier()]);
//        }catch (ParseException&){}
        m_panel_holder.load_panel(descriptor, std::move(panel));

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

void PanelListWidget::set_panel(const std::string& panel_name){
    const auto panels = findItems(QString::fromStdString(panel_name), Qt::MatchExactly);
    if (panels.size() > 0){
        setCurrentItem(panels[0]);
    }

    handle_panel_clicked(panel_name);
}




}
