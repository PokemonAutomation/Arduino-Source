/*  Resource Download Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QFont>
#include <QObject>
#include <QPushButton>
#include <QPointer>
#include "CommonFramework/Logging/Logger.h"

#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "ResourceDownloadWidget.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


template class RegisterConfigWidget<DownloadButtonWidget>;

DownloadButtonWidget::~DownloadButtonWidget(){
}
DownloadButtonWidget::DownloadButtonWidget(QWidget& parent, ResourceDownloadButton& value)
    : QWidget(&parent)
    , ConfigWidget(value)
    , m_value(value)
{
    QPushButton* button = new QPushButton(&parent);
    m_widget = button;

    QFont font;
    font.setBold(true);
    button->setFont(font);

    // Button should be disabled when in the middle of downloading
    // this status is stored within ResourceDownloadButton::m_enabled
    // when the button is clicked, m_enabled is set to false
    // when te download is done, m_enabled is set back to true
    // the UI is updated to reflect the status of m_enabled, by using update_enabled_status
    auto update_enabled_status = [this, button](){
        if (m_value.get_enabled()){
            button->setEnabled(true);
            button->setText("Download");
        }else{
            button->setEnabled(false);
            button->setText("Downloading...");
        }
    };

    // update the UI based on m_enabled when the button is constructed
    update_enabled_status();

    // when the button is clicked, m_enabled is set to false, the UI is updated, and we run_download()
    connect(
        button, &QPushButton::clicked,
        this, [this, update_enabled_status](){
            m_value.set_enabled(false);
            update_enabled_status();
            m_value.run_download();
        }
    );

    // when the download is finished, update the UI
    connect(
        &m_value, &ResourceDownloadButton::download_finished,
        this, update_enabled_status
    );
}


template class RegisterConfigWidget<DeleteButtonWidget>;


DeleteButtonWidget::DeleteButtonWidget(QWidget& parent, ResourceDeleteButton& value)
    : ConfigWidget(value)
{
    QPushButton* button = new QPushButton(&parent);
    m_widget = button;

    QFont font;
    font.setBold(true);
    button->setFont(font);
    button->setText("Delete");

    button->connect(
        button, &QPushButton::clicked,
        button, [&](bool){
            cout << "Clicked Delete Button" << endl;
        }
    );
}



}
