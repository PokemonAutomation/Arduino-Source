/*  Required Download Dialog Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QVBoxLayout>
#include <QMessageBox>
// #include <QFont>
// #include <QObject>
// #include <QPushButton>
// #include <QPointer>
// #include <QHBoxLayout>
// #include "CommonFramework/Logging/Logger.h"
#include "Common/Cpp/Exceptions.h"

// #include "CommonFramework/Notifications/ProgramNotifications.h"
#include "RequiredDownloadDialogWidget.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


////////////////////////////////////////////
// RequiredDownloadWidget
////////////////////////////////////////////

RequiredDownloadWidget::~RequiredDownloadWidget(){
    m_value->remove_listener(*this);
}

RequiredDownloadWidget::RequiredDownloadWidget(QWidget& parent, std::shared_ptr<RequiredDownload> download_ptr)
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

    download_ptr->add_listener(*this);
}


void RequiredDownloadWidget::update_progress_bar(uint64_t bytes_done, uint64_t total_bytes, const std::string& text){
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

void RequiredDownloadWidget::on_download_progress(uint64_t bytes_done, uint64_t total_bytes){
    QMetaObject::invokeMethod(this, [this, bytes_done, total_bytes]{
        update_progress_bar(bytes_done, total_bytes, "Downloading");        
    }, Qt::QueuedConnection);
}
void RequiredDownloadWidget::on_unzip_progress(uint64_t bytes_done, uint64_t total_bytes){
    QMetaObject::invokeMethod(this, [this, bytes_done, total_bytes]{
        update_progress_bar(bytes_done, total_bytes, "Unzipping");        
    }, Qt::QueuedConnection);
}
void RequiredDownloadWidget::on_hash_progress(uint64_t bytes_done, uint64_t total_bytes){
    QMetaObject::invokeMethod(this, [this, bytes_done, total_bytes]{
        update_progress_bar(bytes_done, total_bytes, "Verifying");        
    }, Qt::QueuedConnection);
}

void RequiredDownloadWidget::on_download_failed(){
    std::cerr << "Error: Download failed. Check your internet connection and check you have enough disk space." << std::endl;
    QMetaObject::invokeMethod(this, []{
        QMessageBox box;
        box.critical(nullptr, "Error", 
            QString::fromStdString("Error: Download failed. Check your internet connection and check you have enough disk space."));
    });
}

////////////////////////////////////////////
// RequiredDownloadDialogWidget
////////////////////////////////////////////

RequiredDownloadDialogWidget::~RequiredDownloadDialogWidget(){
    m_download_manager.remove_download_listener(*this);
}
RequiredDownloadDialogWidget::RequiredDownloadDialogWidget(QWidget& parent, RequiredDownloadManager& download_manager)
    : QDialog (&parent)
    , m_download_manager(download_manager)
{
    setWindowTitle("Task Progress");
    resize(400, 50 * (int)download_manager.get_required_downloads().size() + 50); // Dynamically scale height

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    for (std::shared_ptr<RequiredDownload> download_ptr : download_manager.get_required_downloads()){
        // cout << download_ptr->get_name() << endl;

        RequiredDownloadWidget* download_layout = new RequiredDownloadWidget(*this, download_ptr);
        download_ptr->start_download();

        mainLayout->addWidget(download_layout);

        // Add a small spacing gap between tasks
        mainLayout->addSpacing(0);
    }

    // 2. Create standard Dialog Buttons (OK and Cancel)
    QDialogButtonBox* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Cancel, 
        this
    );

    // 3. Connect the buttons to the default QDialog slots
    // Clicking OK calls accept(), clicking Cancel calls reject()
    // connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    // 4. Add the buttons to the bottom of the layout
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    download_manager.add_download_listener(*this);
}


void RequiredDownloadDialogWidget::on_all_downloads_finished(){
    cout << "All downloads finished. Start the program." << endl;
    QMetaObject::invokeMethod(this, [this](){
        this->accept(); 
    });
}
void RequiredDownloadDialogWidget::on_download_failed(){
    std::cerr << "Error: Download failed. Check your internet connection and check you have enough disk space." << std::endl;
    QMetaObject::invokeMethod(this, [this]{
        this->reject();
        QMessageBox box;
        box.critical(nullptr, "Error", 
            QString::fromStdString("Error: Download failed. Check your internet connection and check you have enough disk space."));
    });
    
}
void RequiredDownloadDialogWidget::on_exception_caught(const std::string& function_name){
    std::cerr << "Error: Exception thrown in thread. From " + function_name + ". Report this as a bug." << std::endl;
    QMetaObject::invokeMethod(this, [this, function_name]{
        this->reject();

        QMessageBox box;
        box.critical(nullptr, "Error", 
            QString::fromStdString("Error: Exception thrown in thread. From " + function_name + ". Report this as a bug."));
    });
}

/////////////////////////
// Non member functions
/////////////////////////


bool show_download_prereqs_popup(
    QWidget* parent, 
    RequiredDownloadManager& download_manager, 
    std::function<void(const std::string&)> error_callback
){

    try{

        // RequiredDownloadManager& download_manager = m_session.get_download_manager();
        // re-initialize the required downloads, even if already initialized
        // this refreshes the download list, which may have changed since the last run
        download_manager.initialize_required_downloads();  

        if (download_manager.get_upgrade_warning()){
            std::string warning_string = 
                "The program is expecting an older version of a resource than is available. "
                "This likely means that your version of Computer Control is out of date. "
                "We recommend that you upgrade the Computer Control program.";
            error_callback(warning_string);
            // cout << warning_string << endl;
        }
        if (download_manager.get_required_downloads().empty()){
            cout << "required_download_list is empty. Start the program." << endl;

            return true;
        }

        RequiredDownloadDialogWidget box{*parent, download_manager};

    //    box.open();
        if (box.exec() == QDialog::Accepted){
            cout << "Pre-req downloads done" << endl;
            return true;
        }else{
            cout << "Pre-req downloads NOT done." << endl;
            download_manager.cancel_downloads();
            return false;
        }
            
    }catch(InternalProgramError& e){
        error_callback(e.message());
    }catch (const std::exception& e) {
        std::string message = std::string(e.what()) + "Report this as an error.";
        error_callback(message);
    }catch(...){
        error_callback("show_download_prereqs_popup: Unknown exception caught. Report this as an error.");
    }

    return false;

}




}
