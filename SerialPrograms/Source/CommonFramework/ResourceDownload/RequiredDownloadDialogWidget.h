/*  Required Download Dialog Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_RequiredDownloadDialogWidget_H
#define PokemonAutomation_RequiredDownloadDialogWidget_H

#include <QDialog>
#include <QLabel>
#include <QProgressBar>
#include "RequiredDownloadManager.h"
#include "CommonFramework/ResourceDownload/RequiredDownload.h"


namespace PokemonAutomation{


class RequiredDownloadWidget : public QWidget, public RequiredDownload::Listener {
public:
    ~RequiredDownloadWidget();
    RequiredDownloadWidget(QWidget& parent, std::shared_ptr<RequiredDownload> download_ptr);

public:
    virtual void on_download_progress(uint64_t bytes_done, uint64_t total_bytes) override;
    virtual void on_unzip_progress(uint64_t bytes_done, uint64_t total_bytes) override;
    virtual void on_hash_progress(uint64_t bytes_done, uint64_t total_bytes) override;

    virtual void on_download_failed() override;


    void update_progress_bar(uint64_t bytes_done, uint64_t total_bytes, const std::string& text);

private:
    std::shared_ptr<RequiredDownload> m_value;
    QLabel* m_resource_name;
    QLabel* m_status_label;
    QProgressBar* m_progress_bar;

};

class RequiredDownloadDialogWidget : public QDialog, public RequiredDownloadManager::DownloadListener {
public:

public:
    ~RequiredDownloadDialogWidget();
    RequiredDownloadDialogWidget(QWidget& parent, RequiredDownloadManager& download_manager);

public:
    virtual void on_all_downloads_finished() override;
    virtual void on_download_failed() override;
    virtual void on_exception_caught(const std::string& function_name) override;

    // virtual void on_action_state_updated() override;

private:    
    // void update_UI_state();
    // void update_progress_bar(int percentage, const std::string& text);
    // void update_progress_bar(uint64_t bytes_done, uint64_t total_bytes, const std::string& text);
private:
    RequiredDownloadManager& m_download_manager;
    
};

// Opens a pop-up box that downloads all the pre-requisite resources
// that haven't yet been downloaded.
// return true if all pre-requisite resources are downloaded
bool show_download_prereqs_popup(
    QWidget* parent, 
    RequiredDownloadManager& download_manager, 
    std::function<void(const std::string&)> error_callback
);




}
#endif
