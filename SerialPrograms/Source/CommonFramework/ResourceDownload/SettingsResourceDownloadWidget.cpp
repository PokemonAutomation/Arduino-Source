/*  Resource Download Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QFont>
#include <QPushButton>
#include <QMessageBox>
#include <QHBoxLayout>
#include "CommonFramework/Logging/Logger.h"
#include "Common/Cpp/Exceptions.h"
#include "SettingsResourceDownloadWidget.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


/////////////////////////////////////////////////////////////////////////////////////////////////////////
// SettingsDownloadButtonWidget
/////////////////////////////////////////////////////////////////////////////////////////////////////////

template class RegisterConfigWidget<SettingsDownloadButtonWidget>;
SettingsDownloadButtonWidget::~SettingsDownloadButtonWidget(){
    // cout << "Destructor for SettingsDownloadButtonWidget" << endl;
    // m_value.disconnect(this);
    m_value.remove_button_listener(*this);
}
SettingsDownloadButtonWidget::SettingsDownloadButtonWidget(QWidget& parent, SettingsResourceDownloadButton& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
    , m_value(value)
    , m_row(value.row)
{

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_button = new QPushButton("Download", this);
    // m_widget = this;

    layout->addWidget(m_button);

    // cout << "Constructor for SettingsDownloadButtonWidget" << endl;

    QFont font;
    font.setBold(true);
    m_button->setFont(font);
    m_button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QFontMetrics metrics(m_button->font());
    int minWidth = metrics.horizontalAdvance("Downloading...");
    m_button->setMinimumWidth(minWidth);


    // when the button is clicked, runs row.update_action_state(), which updates the button state
    // also, fetch json
    connect(
        m_button, &QPushButton::clicked,
        this, [this](){
            if (!m_row.is_given_action_state(ActionState::READY)){
                return;
            }
            m_row.update_action_state(ActionState::PRE_DOWNLOAD);
            m_row.ensure_remote_metadata_loaded();
        }
    );


    // add QObject that manages the Download confirmation popup
    SettingsDownloadPopupWidget* popup = new SettingsDownloadPopupWidget(m_row);
    popup->setParent(this);

    value.add_button_listener(*this);
}


void SettingsDownloadButtonWidget::on_change_text(const std::string& text){
    QMetaObject::invokeMethod(this, [this, text]{
        m_button->setText(QString::fromStdString(text));
    }, Qt::QueuedConnection);
}




/////////////////////////////////////////////////////////////////////////////////////////////////////////
// SettingsDeleteButtonWidget
/////////////////////////////////////////////////////////////////////////////////////////////////////////

template class RegisterConfigWidget<SettingsDeleteButtonWidget>;
SettingsDeleteButtonWidget::~SettingsDeleteButtonWidget(){
    m_value.remove_button_listener(*this);
}
SettingsDeleteButtonWidget::SettingsDeleteButtonWidget(QWidget& parent, SettingsResourceDeleteButton& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
    , m_value(value)
    , m_row(value.row)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_button = new QPushButton("Delete", this);
    // m_widget = m_button;

    layout->addWidget(m_button);

    QFont font;
    font.setBold(true);
    m_button->setFont(font);
    m_button->setText("Delete");
    m_button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QFontMetrics metrics(m_button->font());
    int minWidth = metrics.horizontalAdvance("Deleting...");
    m_button->setMinimumWidth(minWidth);


    // when the button is clicked, runs row.update_action_state(), which updates the button state
    // also, show the delete confirm box
    connect(
        m_button, &QPushButton::clicked,
        this, [&](bool){
            if (!m_row.is_given_action_state(ActionState::READY)){
                return;
            }
            m_row.update_action_state(ActionState::PRE_DELETE);
            show_delete_confirm_box();
            // cout << "Clicked Delete Button" << endl;
        }
    );

    value.add_button_listener(*this);
}

void SettingsDeleteButtonWidget::on_change_text(const std::string& text){
    QMetaObject::invokeMethod(this, [this, text]{
        m_button->setText(QString::fromStdString(text));
    }, Qt::QueuedConnection);
}


void SettingsDeleteButtonWidget::show_delete_confirm_box(){
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

        m_row.start_delete();
        return;
    }
    if (clicked == cancel){
        m_row.update_action_state(ActionState::READY);
        return;
    }
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////
// SettingsCancelButtonWidget
/////////////////////////////////////////////////////////////////////////////////////////////////////////

template class RegisterConfigWidget<SettingsCancelButtonWidget>;
SettingsCancelButtonWidget::~SettingsCancelButtonWidget(){
    m_value.remove_button_listener(*this);
}
SettingsCancelButtonWidget::SettingsCancelButtonWidget(QWidget& parent, SettingsResourceCancelButton& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
    , m_value(value)
    , m_row(value.row)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_button = new QPushButton("Cancel", this);
    // m_widget = m_button;

    layout->addWidget(m_button);

    QFont font;
    font.setBold(true);
    m_button->setFont(font);
    m_button->setText("Cancel");
    m_button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QFontMetrics metrics(m_button->font());
    int minWidth = metrics.horizontalAdvance("Cancelling...");
    m_button->setMinimumWidth(minWidth);

    // when the button is clicked, runs row.update_action_state(), which updates the button state
    // also, set cancel state to true
    connect(
        m_button, &QPushButton::clicked,
        this, [&](bool){
            if (!m_row.is_given_action_state(ActionState::DOWNLOADING)){
                return;
            }
            m_row.update_action_state(ActionState::PRE_CANCEL);
            show_cancel_confirm_box();
            cout << "Clicked Cancel Button" << endl;
        }
    );

    value.add_button_listener(*this);
}

void SettingsCancelButtonWidget::on_change_text(const std::string& text){
    QMetaObject::invokeMethod(this, [this, text]{
        m_button->setText(QString::fromStdString(text));
    }, Qt::QueuedConnection);
}

void SettingsCancelButtonWidget::show_cancel_confirm_box(){
    QMessageBox box;
    QPushButton* yes = box.addButton(QMessageBox::Yes);
    QPushButton* cancel = box.addButton("Cancel", QMessageBox::NoRole);
    box.setEscapeButton(cancel);
//    cout << "ok = " << ok << endl;
//    cout << "skip = " << skip << endl;

    box.setTextFormat(Qt::RichText);
    std::string title = "Cancel Download";
    std::string message_body = "Are you sure you want to cancel this download?";

    box.setWindowTitle(QString::fromStdString(title));
    box.setText(QString::fromStdString(message_body));

//    box.open();

    box.exec();

    QAbstractButton* clicked = box.clickedButton();
//    cout << "clicked = " << clicked << endl;
    if (clicked == yes){
        cout << "Clicked Yes to Cancel" << endl;

        if (!m_row.is_given_action_state(ActionState::PRE_CANCEL)){ 
            // if the download finishes and goes back to READY state before the user clicks Yes to cancel
            // then nothing happens
            return;
        }

        m_row.update_action_state(ActionState::CANCELLING);

        m_row.cancel_download_thread();

        return;
    }
    if (clicked == cancel){
        if (!m_row.is_given_action_state(ActionState::PRE_CANCEL)){
            return;
        }

        m_row.update_action_state(ActionState::DOWNLOADING);
        return;
    }
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////
// SettingsProgressBarWidget
/////////////////////////////////////////////////////////////////////////////////////////////////////////

template class RegisterConfigWidget<SettingsProgressBarWidget>;
SettingsProgressBarWidget::~SettingsProgressBarWidget(){
    // cout << "Destructor for SettingsProgressBarWidget" << endl;
    m_value.remove_progress_listener(*this);
}
SettingsProgressBarWidget::SettingsProgressBarWidget(QWidget& parent, SettingsResourceProgressBar& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
    , m_value(value)
    // , m_row(value.row)
{

    // 1. Instantiate the widgets
    m_status_label = new QLabel("", this);
    m_progress_bar = new QProgressBar(this);

    // cout << "Constructor for SettingsProgressBarWidget" << endl;

    // 2. Configure the progress bar
    m_progress_bar->setRange(0, 100);
    m_progress_bar->setValue(0);
    m_progress_bar->setTextVisible(true); // Shows % inside the bar

    // 3. Create a horizontal layout to hold them
    QHBoxLayout *layout = new QHBoxLayout();
    layout->addWidget(m_status_label);
    layout->addWidget(m_progress_bar);

    this->setLayout(layout);
    this->setMinimumWidth(170);

    value.add_progress_listener(*this);

    update_visibility();
}

void SettingsProgressBarWidget::update_visibility(){
    ConfigWidget::update_visibility();

    switch (m_value.visibility()){
    case ConfigOptionState::ENABLED:
    case ConfigOptionState::DISABLED:
        m_progress_bar->setVisible(true);
        break;
    case ConfigOptionState::HIDDEN:
        m_progress_bar->setVisible(false);
        break;
    }

}


void SettingsProgressBarWidget::update_progress_bar(int percentage){
    m_progress_bar->setValue(percentage);
}

void SettingsProgressBarWidget::update_progress_bar(uint64_t bytes_done, uint64_t total_bytes){
    double percent = total_bytes > 0 ? (static_cast<double>(bytes_done) / total_bytes) * 100.0 : 0;
    int current_percent = static_cast<int>(percent);
    int last_percentage = m_progress_bar->value();
    // Only update UI if integer value has changed
    if (current_percent != last_percentage){
        update_progress_bar(current_percent);
    }
}

void SettingsProgressBarWidget::on_change_text(const std::string& text){
    QMetaObject::invokeMethod(this, [this, text]{
        m_status_label->setText(QString::fromStdString(text));
    }, Qt::QueuedConnection);

}
void SettingsProgressBarWidget::on_update_progress(uint64_t bytes_done, uint64_t total_bytes){
    QMetaObject::invokeMethod(this, [this, bytes_done, total_bytes]{
        update_progress_bar(bytes_done, total_bytes);
    }, Qt::QueuedConnection);
}

void SettingsProgressBarWidget::on_reset_progress(){
    QMetaObject::invokeMethod(this, [this]{
        m_progress_bar->setValue(0);
    }, Qt::QueuedConnection);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// SettingsDownloadPopupWidget
/////////////////////////////////////////////////////////////////////////////////////////////////////////
SettingsDownloadPopupWidget::~SettingsDownloadPopupWidget(){
    m_row.remove_listener(*this);
}
SettingsDownloadPopupWidget::SettingsDownloadPopupWidget(SettingsResourceDownloadRow& row)
    : m_row(row)
{
    m_row.add_listener(*this);
}

void SettingsDownloadPopupWidget::show_download_confirm_box(
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

        m_row.start_download();
        return;
    }
    if (clicked == cancel){
        m_row.update_action_state(ActionState::READY);
        return;
    }
}

// when json has been fetched, open the update box. 
// When click Ok in update box, start the download. If click cancel, re-enable the download button
void SettingsDownloadPopupWidget::on_metadata_fetch_finished(const std::string& popup_message){
    QMetaObject::invokeMethod(this, [this, popup_message]{
        show_download_confirm_box("Download", popup_message);
    }, Qt::QueuedConnection);

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// SettingsDownloadErrorWidget
/////////////////////////////////////////////////////////////////////////////////////////////////////////
template class RegisterConfigWidget<SettingsDownloadErrorWidget>;
SettingsDownloadErrorWidget::~SettingsDownloadErrorWidget(){
    GlobalResourceDownloadManager::instance().remove_download_listener(*this);
}
SettingsDownloadErrorWidget::SettingsDownloadErrorWidget(QWidget& parent, SettingsDownloadError& value)
    : QWidget(&parent)
    , ConfigWidget(value, *this)
{
    GlobalResourceDownloadManager::instance().add_download_listener(*this);
    this->hide();
}

void SettingsDownloadErrorWidget::show_download_failed_box(const std::string& resource_slug){
    std::cerr << "SettingsDownloadErrorWidget::show_download_failed_box: Error: Download failed for " << resource_slug << ". Check your internet connection and check you have enough disk space." << std::endl;
    if (m_popup_is_open.exchange(true)){ // only show popups if one isn't already open
        return;
    }

    QMessageBox box;
    box.warning(nullptr, "Error:",
        QString::fromStdString("Error: Download failed for " + resource_slug + " (and others possibly as well). Check your internet connection and check you have enough disk space."));

    m_popup_is_open.store(false); 
}

void SettingsDownloadErrorWidget::show_error_box(std::string error_msg){
    std::cerr << "SettingsDownloadErrorWidget::show_error_box: " << error_msg << std::endl;
    if (m_popup_is_open.exchange(true)){
        return;
    }    
    QMessageBox box;
    box.warning(nullptr, "Error:",
        QString::fromStdString(error_msg));

    m_popup_is_open.store(false); 

}

// GlobalResourceDownloadManager::Listener
//
void SettingsDownloadErrorWidget::on_download_failed(const std::string& resource_slug){
    QMetaObject::invokeMethod(this, [this, resource_slug]{
        show_download_failed_box(resource_slug);
    }, Qt::QueuedConnection);    
}
void SettingsDownloadErrorWidget::on_exception_caught(const std::string& error_msg){
    QMetaObject::invokeMethod(this, [this, error_msg]{
        show_error_box(error_msg);
    }, Qt::QueuedConnection);
}

}
