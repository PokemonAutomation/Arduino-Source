/*  Settings Panel
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QVBoxLayout>
#include <QPushButton>
#include <QScrollArea>
#include <QGroupBox>
#include <QMessageBox>
#include "Common/Qt/CollapsibleGroupBox.h"
#include "Common/Qt/Options/BatchWidget.h"
#include "SettingsPanelWidget.h"

namespace PokemonAutomation{


QWidget* SettingsPanelInstance::make_widget(QWidget& parent, PanelHolder& holder){
    return SettingsPanelWidget::make(parent, *this, holder);
}


SettingsPanelWidget* SettingsPanelWidget::make(
    QWidget& parent,
    SettingsPanelInstance& instance,
    PanelHolder& holder
){
    SettingsPanelWidget* widget = new SettingsPanelWidget(parent, instance, holder);
    widget->construct();
    return widget;
}
SettingsPanelWidget::SettingsPanelWidget(
    QWidget& parent,
    SettingsPanelInstance& instance,
    PanelHolder& holder
)
    : PanelWidget(parent, instance, holder)
{}
void SettingsPanelWidget::construct(){
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
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
    m_options = static_cast<BatchWidget*>(instance.m_options.make_QtWidget(parent));
    options_layout->addWidget(m_options);
    options_layout->addStretch();

    return options_widget;
}
QWidget* SettingsPanelWidget::make_actions(QWidget& parent){
    QGroupBox* actions_widget = new QGroupBox("Actions", &parent);

    QHBoxLayout* action_layout = new QHBoxLayout(actions_widget);
//    action_layout->setContentsMargins(0, 0, 0, 0);
    {
        m_default_button = new QPushButton("Restore Defaults", actions_widget);
        action_layout->addWidget(m_default_button, 1);
        QFont font = m_default_button->font();
        font.setPointSize(16);
        m_default_button->setFont(font);
    }

    connect(
        m_default_button, &QPushButton::clicked,
        this, [this](bool){
            QMessageBox::StandardButton button = QMessageBox::question(
                nullptr,
                "Restore Defaults",
                "Are you sure you wish to restore settings back to defaults? This will wipe the current settings.",
                QMessageBox::Ok | QMessageBox::Cancel
            );
            if (button == QMessageBox::Ok){
                restore_defaults();
            }
        }
    );

    return actions_widget;
}

void SettingsPanelWidget::restore_defaults(){
    m_options->option().restore_defaults();
}



}
