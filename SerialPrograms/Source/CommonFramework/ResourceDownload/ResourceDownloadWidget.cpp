/*  Resource Download Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QFont>
#include <QObject>
#include <QPushButton>
#include <QMessageBox>
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
    , m_button(new QPushButton(&parent))
{
    m_widget = m_button;

    QFont font;
    font.setBold(true);
    m_button->setFont(font);

    // Button should be disabled when in the middle of downloading
    // this status is stored within ResourceDownloadButton::m_enabled
    // when the button is clicked, m_enabled is set to false
    // when te download is done, m_enabled is set back to true
    // the UI is updated to reflect the status of m_enabled, by using update_enabled_status


    // update the UI based on m_enabled, when the button is constructed
    update_enabled_status();

    // when the button is clicked, m_enabled is set to false,
    // fetch json
    connect(
        m_button, &QPushButton::clicked,
        this, [this](){
            m_value.set_enabled(false);
            update_enabled_status();
            m_value.fetch_remote_metadata();
        }
    );

    // when json has been fetched, open the update box. 
    // When click Ok in update box, start the download. If click cancel, re-enable the download button
    connect(
        &m_value, &ResourceDownloadButton::metadata_fetch_finished,
        this, [this](){
            show_download_confirm_box("Download", "Download", "body");
        }
    );

    // when the download is finished, update the UI to re-enable the button
    connect(
        &m_value, &ResourceDownloadButton::download_finished,
        this, &DownloadButtonWidget::update_enabled_status
    );
}


void DownloadButtonWidget::update_enabled_status(){
    if (m_value.get_enabled()){
        m_button->setEnabled(true);
        m_button->setText("Download");
    }else{
        m_button->setEnabled(false);
        m_button->setText("Downloading...");
    }
}


void DownloadButtonWidget::show_download_confirm_box(
    const std::string& title,
    const std::string& header,
    const std::string& message_body
){
    QMessageBox box;
    QPushButton* ok = box.addButton(QMessageBox::Ok);
    QPushButton* cancel = box.addButton("Cancel", QMessageBox::NoRole);
    box.setEscapeButton(cancel);
//    cout << "ok = " << ok << endl;
//    cout << "skip = " << skip << endl;

    box.setTextFormat(Qt::RichText);
    std::string text = header + "<br>";
    // text += make_text_url(link_url, link_text);
    // text += get_changes(node);


    box.setWindowTitle(QString::fromStdString(title));
    box.setText(QString::fromStdString(text));

//    box.open();

    box.exec();

    QAbstractButton* clicked = box.clickedButton();
//    cout << "clicked = " << clicked << endl;
    if (clicked == ok){
        cout << "Clicked Ok to Download" << endl;

        m_value.run_download();
        return;
    }
    if (clicked == cancel){
        m_value.set_enabled(true);
        update_enabled_status();
        return;
    }
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
