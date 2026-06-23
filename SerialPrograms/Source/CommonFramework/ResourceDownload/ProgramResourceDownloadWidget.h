/*  Required Download Dialog Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ProgramResourceDownloadWidget_H
#define PokemonAutomation_ProgramResourceDownloadWidget_H

#include <QDialog>
#include <QLabel>
#include <QVBoxLayout>
#include <QProgressBar>
#include "ProgramMissingResourceTracker.h"
#include "GlobalResourceDownloadManager.h"
#include "CommonFramework/ResourceDownload/ResourceDownload.h"


namespace PokemonAutomation{

class ProgramSession;

class ProgramResourceDownloadRowWidget : public QWidget, public ResourceDownload::Listener {
public:
    ~ProgramResourceDownloadRowWidget();
    ProgramResourceDownloadRowWidget(QWidget& parent, std::shared_ptr<ResourceDownload> download_ptr);

public:
    virtual void on_download_progress(uint64_t bytes_done, uint64_t total_bytes) override;
    virtual void on_unzip_progress(uint64_t bytes_done, uint64_t total_bytes) override;
    virtual void on_hash_progress(uint64_t bytes_done, uint64_t total_bytes) override;

    // virtual void on_download_failed(const std::string& resource_slug) override;


    void update_progress_bar(uint64_t bytes_done, uint64_t total_bytes, const std::string& text);

private:
    std::shared_ptr<ResourceDownload> m_value;
    QLabel* m_resource_name;
    QLabel* m_status_label;
    QProgressBar* m_progress_bar;

};

class ProgramResourceDownloadTableWidget : public QWidget {

public:
    ~ProgramResourceDownloadTableWidget();
    ProgramResourceDownloadTableWidget(QWidget& parent);

    void add_download(std::shared_ptr<ResourceDownload> download_ptr);

    void remove_all_downloads();

private:
    QVBoxLayout* m_layout;
    
};




}
#endif
