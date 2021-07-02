/*  Settings Panel
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QJsonObject>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QGroupBox>
#include "Common/Qt/QtJsonTools.h"
#include "SettingsPanel.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{



void SettingsPanelInstance::from_json(const QJsonValue& json){
//    cout << QJsonDocument(json.toObject()).toJson().data() << endl;
//    QJsonObject obj = json_get_object_nothrow(json.toObject(), m_descriptor.name());
    QJsonObject obj = json.toObject();
    for (auto& item : m_options){
        if (!item.first.isEmpty()){
            item.second->load_json(json_get_value_nothrow(obj, item.first));
        }
    }
}
QJsonValue SettingsPanelInstance::to_json() const{
    QJsonObject obj;
    for (auto& item : m_options){
        if (!item.first.isEmpty()){
            obj.insert(item.first, item.second->to_json());
        }
    }
    return obj;
}
QWidget* SettingsPanelInstance::make_widget(QWidget& parent, PanelListener& listener){
    return SettingsPanelWidget::make(parent, *this, listener);
}



SettingsPanelWidget* SettingsPanelWidget::make(
    QWidget& parent,
    SettingsPanelInstance& instance,
    PanelListener& listener
){
    SettingsPanelWidget* widget = new SettingsPanelWidget(parent, instance, listener);
    widget->construct();
    return widget;
}
SettingsPanelWidget::SettingsPanelWidget(
    QWidget& parent,
    SettingsPanelInstance& instance,
    PanelListener& listener
)
    : PanelWidget(parent, instance, listener)
{}
void SettingsPanelWidget::construct(){
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->addWidget(make_header(*this));

    QScrollArea* scroll = new QScrollArea(this);
    layout->addWidget(scroll);
    scroll->setWidgetResizable(true);

    scroll->setWidget(make_options(*scroll));
    layout->addWidget(make_actions(*this));
}
QWidget* SettingsPanelWidget::make_options(QWidget& parent){
    QWidget* options_widget = new QWidget(&parent);
    (new QVBoxLayout(&parent))->addWidget(options_widget);

    QVBoxLayout* options_layout = new QVBoxLayout(options_widget);
    options_layout->setAlignment(Qt::AlignTop);


    SettingsPanelInstance& instance = static_cast<SettingsPanelInstance&>(m_instance);
    for (auto& item : instance.m_options){
        m_options.emplace_back(item.second->make_ui(*options_widget));
        options_layout->addWidget(m_options.back()->widget());
    }

    return options_widget;
}
QWidget* SettingsPanelWidget::make_actions(QWidget& parent){
    QGroupBox* actions_widget = new QGroupBox("Actions", &parent);

    QHBoxLayout* action_layout = new QHBoxLayout(actions_widget);
    action_layout->setMargin(0);
    {
        m_default_button = new QPushButton("Restore Defaults", actions_widget);
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

    return actions_widget;
}

void SettingsPanelWidget::restore_defaults(){
    for (ConfigOptionUI* item : m_options){
        item->restore_defaults();
    }
}




}
