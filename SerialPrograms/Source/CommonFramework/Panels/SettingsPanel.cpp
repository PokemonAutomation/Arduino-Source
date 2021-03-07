/*  Settings Panel
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <iostream>
#include <QJsonObject>
#include <QScrollArea>
#include <QGroupBox>
#include "Common/Qt/StringException.h"
#include "Common/Qt/QtJsonTools.h"
#include "SettingsPanel.h"

using std::cout;
using std::endl;

namespace PokemonAutomation{


void SettingsPanel::from_json(const QJsonValue& json){
    try{
        const QJsonObject& obj = json_get_object(json_cast_object(json), m_name);
        for (auto& item : m_options){
            if (!item.first.isEmpty()){
                QJsonValue value = json_get_value(obj, item.first);
                item.second->load_json(value);
            }
        }
    }catch (const StringException& str){
        cout << str.message().toUtf8().data() << endl;
    }catch (...){
        cout << "Failed to catch." << endl;
    }
}
QJsonValue SettingsPanel::to_json() const{
    QJsonObject obj;
    for (auto& item : m_options){
        if (!item.first.isEmpty()){
            obj.insert(item.first, item.second->to_json());
        }
    }
    return obj;
}


QWidget* SettingsPanel::make_ui(MainWindow& window){
    SettingsPanelUI* widget = new SettingsPanelUI(*this);
    widget->construct();
    return widget;
}

SettingsPanelUI::SettingsPanelUI(SettingsPanel& factory)
    : RightPanelUI(factory)
    , m_factory(factory)
{}



void SettingsPanelUI::make_body(QWidget& parent, QVBoxLayout& layout){
    QScrollArea* scroll = new QScrollArea(&parent);
    layout.addWidget(scroll);
    scroll->setWidgetResizable(true);

    QWidget* options_widget = new QWidget(scroll);
    (new QVBoxLayout(scroll))->addWidget(options_widget);
    scroll->setWidget(options_widget);

    QVBoxLayout* options_layout = new QVBoxLayout(options_widget);
    options_layout->setAlignment(Qt::AlignTop);


    for (auto& item : m_factory.m_options){
        m_options.emplace_back(item.second->make_ui(parent));
        options_layout->addWidget(m_options.back()->widget());
    }


    QGroupBox* actions_widget = new QGroupBox("Actions", &parent);
    layout.addWidget(actions_widget);

    QHBoxLayout* action_layout = new QHBoxLayout(actions_widget);
    action_layout->setMargin(0);
    {
        m_default_button = new QPushButton("Restore Defaults", &parent);
        action_layout->addWidget(m_default_button, 1);
        QFont font = m_default_button->font();
        font.setPointSize(16);
        m_default_button->setFont(font);
    }

    connect(
        m_default_button, &QPushButton::clicked,
        this, [=](bool){
            restore_defaults();
        }
    );
}

void SettingsPanelUI::restore_defaults(){
    for (ConfigOptionUI* item : m_options){
        item->restore_defaults();
    }
}


}
