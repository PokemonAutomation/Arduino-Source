/*  Required Download Dialog Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

// #include <QVBoxLayout>
#include <QMessageBox>
#include <QEventLoop>
// #include <QFont>
// #include <QObject>
// #include <QPushButton>
// #include <QPointer>
// #include <QHBoxLayout>
// #include "CommonFramework/Logging/Logger.h"
#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/ProgramSession.h"
// #include "CommonFramework/Notifications/ProgramNotifications.h"
#include "ProgramResourceDownloadWidget.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


////////////////////////////////////////////
// ProgramResourceDownloadRowWidget
////////////////////////////////////////////

ProgramResourceDownloadRowWidget::~ProgramResourceDownloadRowWidget(){
    m_value->remove_listener(*this);
}

ProgramResourceDownloadRowWidget::ProgramResourceDownloadRowWidget(QWidget& parent, std::shared_ptr<ResourceDownload> download_ptr)
    : QWidget(&parent)
    , m_value(download_ptr)
{

    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    // Create a label for the specific task
    m_resource_name = new QLabel(QString::fromStdString(download_ptr->get_name()), this);
    m_resource_name->setFixedWidth(300);
    mainLayout->addWidget(m_resource_name);

    // Create a label for the status
    m_status_label = new QLabel("", this);
    m_status_label->setFixedWidth(70);
    mainLayout->addWidget(m_status_label);

    // Create the progress bar
    m_progress_bar = new QProgressBar(this);
    m_progress_bar->setRange(0, 100);
    m_progress_bar->setValue(0);
    m_progress_bar->setFixedWidth(100);
    mainLayout->addWidget(m_progress_bar);

    mainLayout->setContentsMargins(0, 0, 0, 0);

    download_ptr->add_listener(*this);
}


void ProgramResourceDownloadRowWidget::update_progress_bar(uint64_t bytes_done, uint64_t total_bytes, const std::string& text){
    double percent = total_bytes > 0 ? (static_cast<double>(bytes_done) / total_bytes) * 100.0 : 0;
    int current_percent = static_cast<int>(percent);
    int last_percentage = m_progress_bar->value();
    // Only update UI if integer value has changed
    if (current_percent == last_percentage){
        return;
    }

    // current_percent has changed. update the progress bar
    m_status_label->setText(QString::fromStdString(text));
    m_progress_bar->setValue(current_percent);
}

void ProgramResourceDownloadRowWidget::on_download_progress(uint64_t bytes_done, uint64_t total_bytes){
    QMetaObject::invokeMethod(this, [this, bytes_done, total_bytes]{
        update_progress_bar(bytes_done, total_bytes, "Downloading");        
    }, Qt::QueuedConnection);
}
void ProgramResourceDownloadRowWidget::on_unzip_progress(uint64_t bytes_done, uint64_t total_bytes){
    QMetaObject::invokeMethod(this, [this, bytes_done, total_bytes]{
        update_progress_bar(bytes_done, total_bytes, "Unzipping");        
    }, Qt::QueuedConnection);
}
void ProgramResourceDownloadRowWidget::on_hash_progress(uint64_t bytes_done, uint64_t total_bytes){
    QMetaObject::invokeMethod(this, [this, bytes_done, total_bytes]{
        update_progress_bar(bytes_done, total_bytes, "Verifying");        
    }, Qt::QueuedConnection);
}

// void ProgramResourceDownloadRowWidget::on_download_failed(const std::string& resource_slug){
//     std::cerr << "ProgramResourceDownloadRowWidget::on_download_failed: Error" << std::endl;
//     QMetaObject::invokeMethod(this, [resource_slug]{
//         QMessageBox box;
//         box.critical(nullptr, "Error", 
//             QString::fromStdString("Error: Download failed for " + resource_slug + ". Check your internet connection and check you have enough disk space."));
//     });
// }

////////////////////////////////////////////
// ProgramResourceDownloadTableWidget
////////////////////////////////////////////

ProgramResourceDownloadTableWidget::~ProgramResourceDownloadTableWidget(){
    // m_missing_resource_tracker.remove_tracker_listener(*this);
    // GlobalResourceDownloadManager::instance().remove_download_listener(*this);
}
ProgramResourceDownloadTableWidget::ProgramResourceDownloadTableWidget(QWidget& parent)
    : QWidget (&parent)
{
    m_layout = new QVBoxLayout(this);

    setLayout(m_layout);

    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

}

void ProgramResourceDownloadTableWidget::add_download(std::shared_ptr<ResourceDownload> download_ptr){
    ProgramResourceDownloadRowWidget* download_widget = new ProgramResourceDownloadRowWidget(*this, download_ptr);

    m_layout->addWidget(download_widget);

    if (m_layout->count() > 0) {
        this->show();
    }
}


void ProgramResourceDownloadTableWidget::remove_all_downloads(){
    QLayout* current_layout = layout();
    if (!current_layout) return;

    while (current_layout->count() > 0) {
        QLayoutItem* item = current_layout->takeAt(0); // Always take the current front item
        if (QWidget* widget = item->widget()) {
            widget->deleteLater();
        }
        delete item;
    }

    if (m_layout->count() == 0) {
        this->hide();
    }
}




}
