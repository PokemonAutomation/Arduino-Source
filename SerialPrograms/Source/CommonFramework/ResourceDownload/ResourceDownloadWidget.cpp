/*  Resource Download Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QFont>
#include <QPushButton>
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "ResourceDownloadWidget.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


template class RegisterConfigWidget<DownloadButtonWidget>;


DownloadButtonWidget::DownloadButtonWidget(QWidget& parent, ResourceDownloadButton& value)
    : ConfigWidget(value)
{
    QPushButton* button = new QPushButton(&parent);
    m_widget = button;

    QFont font;
    font.setBold(true);
    button->setFont(font);
    button->setText("Download");

    button->connect(
        button, &QPushButton::clicked,
        button, [&](bool){
            cout << "Clicked Download Button" << endl;
        }
    );
}



}
