/*  Discord Integration Settings
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include "Integrations/DppIntegration/DppClient.h"
#include "DiscordIntegrationSettingsWidget.h"

namespace PokemonAutomation{

template class RegisterConfigWidget<Integration::DiscordIntegrationSettingsWidget>;

namespace Integration{



DiscordIntegrationSettingsWidget::DiscordIntegrationSettingsWidget(QWidget& parent, DiscordIntegrationSettingsOption& value)
    : GroupWidget(parent, value)
{
#ifdef PA_DPP

    QWidget* control_buttons = new QWidget(this);
    m_options_layout->insertWidget(0, control_buttons);

    QHBoxLayout* layout = new QHBoxLayout(control_buttons);
    layout->setContentsMargins(5, 5, 5, 5);

    QLabel* text = new QLabel("<b>Bot Control:</b>", control_buttons);
    layout->addWidget(text, 2);
    text->setWordWrap(true);

    QPushButton* button_start = new QPushButton("Start Bot", this);
    layout->addWidget(button_start, 1);

    QPushButton* button_stop = new QPushButton("Stop Bot", this);
    layout->addWidget(button_stop, 1);

    QFont font = button_start->font();
    font.setBold(true);
    button_start->setFont(font);
    button_stop->setFont(font);

    connect(
        button_start, &QPushButton::clicked,
        this, [this, &value](bool){
            DppClient::Client::instance().connect();
            value.on_config_value_changed(this);
        }
    );
    connect(
        button_stop, &QPushButton::clicked,
        this, [this, &value](bool){
            DppClient::Client::instance().disconnect();
            value.on_config_value_changed(this);
        }
    );

#endif
}




}
}
