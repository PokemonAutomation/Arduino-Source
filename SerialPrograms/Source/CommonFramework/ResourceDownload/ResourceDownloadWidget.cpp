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
#include <QHBoxLayout>
#include "CommonFramework/Logging/Logger.h"
#include "Common/Cpp/Exceptions.h"

#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "ResourceDownloadWidget.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{

void show_error_box(std::string function_name){
    std::cerr << "Error: Exception thrown in thread. From " + function_name + ". Report this as a bug." << std::endl;
    QMessageBox box;
    box.warning(nullptr, "Error:",
        QString::fromStdString("Error: Exception thrown in thread. From " + function_name + ". Report this as a bug."));

}

void show_download_failed_box(){
    std::cerr << "Error: Download failed. Check your internet connection." << std::endl;
    QMessageBox box;
    box.warning(nullptr, "Error:",
        QString::fromStdString("Error: Download failed. Check your internet connection."));

}    


template class RegisterConfigWidget<DownloadButtonWidget>;
DownloadButtonWidget::~DownloadButtonWidget(){
    // cout << "Destructor for DownloadButtonWidget" << endl;
    // m_value.disconnect(this);
}
DownloadButtonWidget::DownloadButtonWidget(QWidget& parent, ResourceDownloadButton& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
    , m_value(value)
{
    m_button = new QPushButton(&parent);
    m_widget = m_button;

    // cout << "Constructor for DownloadButtonWidget" << endl;

    QFont font;
    font.setBold(true);
    m_button->setFont(font);

    // Button should be disabled when in the middle of downloading
    // this status is stored within ResourceDownloadButton::m_enabled
    // when the button is clicked, m_enabled is set to false
    // when te download is done, m_enabled is set back to true
    // the UI is updated to reflect the status of m_enabled, by using update_UI_state


    // update the UI based on m_enabled, when the button is constructed
    update_UI_state();

    // when the button is clicked, runs row.update_button_state(), which updates the button state
    // also, fetch json
    connect(
        m_button, &QPushButton::clicked,
        this, [this](){
            m_value.row.update_button_state(ButtonState::DOWNLOAD);
            m_value.row.ensure_remote_metadata_loaded();
        }
    );

    // when button_state_updated, update the UI state to match
    connect(
        &m_value.row, &ResourceDownloadRow::button_state_updated,
        this, [this](){
            update_UI_state();
        }
    );

    // when json has been fetched, open the update box. 
    // When click Ok in update box, start the download. If click cancel, re-enable the download button
    connect(
        &m_value.row, &ResourceDownloadRow::metadata_fetch_finished,
        this, [this](std::string predownload_warning){
            show_download_confirm_box("Download", predownload_warning);
        }
    );

    // if the thread catches an exception, show an error box
    // since exceptions can't bubble up as usual
    // this connect handles all exception_caught() emitted by ResourceDownloadRow
    connect(
        &m_value.row, &ResourceDownloadRow::exception_caught,
        this, [this](std::string function_name){
            show_error_box(function_name);
        }
    );

    // if download fails
    connect(
        &m_value.row, &ResourceDownloadRow::download_failed,
        this, [this](){
            show_download_failed_box();
        }
    );
}


void DownloadButtonWidget::update_UI_state(){
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
    const std::string& message_body
){
    QMessageBox box;
    QPushButton* ok = box.addButton(QMessageBox::Ok);
    QPushButton* cancel = box.addButton("Cancel", QMessageBox::NoRole);
    box.setEscapeButton(cancel);
//    cout << "ok = " << ok << endl;
//    cout << "skip = " << skip << endl;

    box.setTextFormat(Qt::RichText);
    std::string text = message_body;
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

        m_value.row.start_download();
        return;
    }
    if (clicked == cancel){
        m_value.row.update_button_state(ButtonState::READY);
        return;
    }
}





template class RegisterConfigWidget<DeleteButtonWidget>;
DeleteButtonWidget::DeleteButtonWidget(QWidget& parent, ResourceDeleteButton& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
    , m_value(value)
{
    m_button = new QPushButton(&parent);
    m_widget = m_button;

    QFont font;
    font.setBold(true);
    m_button->setFont(font);
    m_button->setText("Delete");


    // update the UI based on m_enabled, when the button is constructed
    update_UI_state();

    // when the button is clicked, runs row.update_button_state(), which updates the button state
    // also, show the delete confirm box
    connect(
        m_button, &QPushButton::clicked,
        this, [&](bool){
            m_value.row.update_button_state(ButtonState::DELETE);
            show_delete_confirm_box();
            cout << "Clicked Delete Button" << endl;
        }
    );

    // when button_state_updated, update the UI state to match
    connect(
        &m_value.row, &ResourceDownloadRow::button_state_updated,
        this, [this](){
            update_UI_state();
        }
    );

}


void DeleteButtonWidget::update_UI_state(){
    if (m_value.get_enabled()){
        m_button->setEnabled(true);
        m_button->setText("Delete");
    }else{
        m_button->setEnabled(false);
        m_button->setText("Deleting...");
    }
}


void DeleteButtonWidget::show_delete_confirm_box(){
    QMessageBox box;
    QPushButton* yes = box.addButton(QMessageBox::Yes);
    QPushButton* cancel = box.addButton("Cancel", QMessageBox::NoRole);
    box.setEscapeButton(cancel);
//    cout << "ok = " << ok << endl;
//    cout << "skip = " << skip << endl;

    box.setTextFormat(Qt::RichText);
    std::string title = "Delete";
    std::string message_body = "Are you sure you want to delete this resource?";

    box.setWindowTitle(QString::fromStdString(title));
    box.setText(QString::fromStdString(message_body));

//    box.open();

    box.exec();

    QAbstractButton* clicked = box.clickedButton();
//    cout << "clicked = " << clicked << endl;
    if (clicked == yes){
        cout << "Clicked Yes to Delete" << endl;

        m_value.row.start_delete();
        return;
    }
    if (clicked == cancel){
        m_value.row.update_button_state(ButtonState::READY);
        return;
    }
}

template class RegisterConfigWidget<CancelButtonWidget>;
CancelButtonWidget::CancelButtonWidget(QWidget& parent, ResourceCancelButton& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
    , m_value(value)
{
    m_button = new QPushButton(&parent);
    m_widget = m_button;

    QFont font;
    font.setBold(true);
    m_button->setFont(font);
    m_button->setText("Cancel");

    // update the UI based on m_enabled, when the button is constructed
    update_UI_state();

    // when the button is clicked, runs row.update_button_state(), which updates the button state
    // also, set cancel state to true
    connect(
        m_button, &QPushButton::clicked,
        this, [&](bool){
            m_value.row.update_button_state(ButtonState::CANCEL);
            cout << "Clicked Cancel Button" << endl;
        }
    );

    // when button_state_updated, update the UI state to match
    connect(
        &m_value.row, &ResourceDownloadRow::button_state_updated,
        this, [this](){
            update_UI_state();
        }
    );

}

void CancelButtonWidget::update_UI_state(){
    if (m_value.get_enabled()){
        m_button->setEnabled(true);
        m_button->setText("Cancel");
    }else{
        m_button->setEnabled(false);
        m_button->setText("Cancelling...");
    }
}

template class RegisterConfigWidget<ProgressBarWidget>;
ProgressBarWidget::~ProgressBarWidget(){
    // m_value.row.disconnect(this);
    // cout << "Destructor for ProgressBarWidget" << endl;

}
ProgressBarWidget::ProgressBarWidget(QWidget& parent, ResourceProgressBar& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
    , m_value(value)
{

    // 1. Instantiate the widgets
    m_status_label = new QLabel("", this);
    m_progress_bar = new QProgressBar(this);

    // cout << "Constructor for ProgressBarWidget" << endl;

    // 2. Configure the progress bar
    m_progress_bar->setRange(0, 100);
    m_progress_bar->setValue(0);
    m_progress_bar->setTextVisible(true); // Shows % inside the bar
    m_progress_bar->hide();

    // 3. Create a horizontal layout to hold them
    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(m_status_label);
    layout->addWidget(m_progress_bar);

    this->setLayout(layout);
    this->setMinimumWidth(170);

    connect(
        &m_value.row, &ResourceDownloadRow::download_progress,
        this, 
        [this](int percentage_progress){
            if (m_progress_bar->isHidden()) {
                m_progress_bar->show(); // Make it visible when progress starts
            }
            m_status_label->setText("Downloading");
            m_progress_bar->setValue(percentage_progress);
            // Simple Console Progress Bar
            // std::cout << "\rProgress: [" << std::string(percentage_progress / 5, '#') 
            //             << std::string(20 - (percentage_progress / 5), ' ') << "] " 
            //             << percentage_progress << "%" << endl;

        }
    );    


    connect(
        &m_value.row, &ResourceDownloadRow::unzip_progress,
        this, 
        [this](int percentage_progress){
            if (m_progress_bar->isHidden()) {
                m_progress_bar->show(); // Make it visible when progress starts
            }
            m_status_label->setText("Unzipping");
            m_progress_bar->setValue(percentage_progress);
        }
    ); 

    // when button_state_updated, update the UI state to match
    connect(
        &m_value.row, &ResourceDownloadRow::button_state_updated,
        this, [this](){
            update_UI_state();
        }
    );
    
}


void ProgressBarWidget::update_UI_state(){
    ButtonState state = m_value.row.get_button_state();
    switch (state){
    case ButtonState::DOWNLOAD:
        m_status_label->setText("Downloading");
        if (m_progress_bar->isHidden()) {
            m_progress_bar->show();
        }
        break;
    case ButtonState::DELETE:
        // m_status_label->setText("");
        // m_progress_bar->hide();
        m_progress_bar->setValue(0);
        break;
    case ButtonState::CANCEL:
        // m_status_label->setText("");
        // m_progress_bar->hide();
        m_progress_bar->setValue(0);
        break;
    case ButtonState::READY:
        m_status_label->setText("");
        m_progress_bar->hide();
        m_progress_bar->setValue(0);
        break;
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "update_UI_state: Unknown enum.");  
    }
}


}
