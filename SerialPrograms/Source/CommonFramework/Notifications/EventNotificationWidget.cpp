/*  Event Notification Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QFont>
#include <QPushButton>
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "EventNotificationWidget.h"

namespace PokemonAutomation{


template class RegisterConfigWidget<TestButtonWidget>;


TestButtonWidget::TestButtonWidget(QWidget& parent, TestMessageButton& value)
    : ConfigWidget(value)
{
    QPushButton* button = new QPushButton(&parent);
    m_widget = button;

    QFont font;
    font.setBold(true);
    button->setFont(font);
    button->setText("Send Test Message");

    button->connect(
        button, &QPushButton::clicked,
        button, [&](bool){
            send_raw_program_notification(
                global_logger_tagged(), value.option,
                COLOR_GREEN,
                ProgramInfo("Test Message"),
                "Notification Test",
                {
                    {"Event Type:", value.option.label()},
                }
            );
        }
    );
}



}
