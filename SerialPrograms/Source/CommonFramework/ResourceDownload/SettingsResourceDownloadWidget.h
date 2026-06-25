/*  Resource Download Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_SettingsResourceDownloadWidget_H
#define PokemonAutomation_SettingsResourceDownloadWidget_H

#include <QObject>
#include <QLabel>
#include <QProgressBar>
#include "Common/Qt/Options/ConfigWidget.h"
// #include "SettingsResourceDownloadTable.h"
#include "GlobalResourceDownloadManager.h"
#include "SettingsResourceDownloadRow.h"

namespace PokemonAutomation{

// class SettingsResourceDownloadButton;

class SettingsDownloadButtonWidget : public QWidget, public ConfigWidget, public SettingsResourceDownloadRow::Listener{
    // Q_OBJECT
public:
    using ParentOption = SettingsResourceDownloadButton;

public:
    ~SettingsDownloadButtonWidget();
    SettingsDownloadButtonWidget(QWidget& parent, SettingsResourceDownloadButton& value);

    virtual void on_metadata_fetch_finished(const std::string& popup_message) override;
    virtual void on_action_state_updated() override;

private:
    void update_UI_state();
    void show_download_confirm_box(
        const std::string& title,
        const std::string& message_body
    );

private:
    SettingsResourceDownloadButton& m_value;
    SettingsResourceDownloadRow& m_row;
    QPushButton* m_button;
    
};

void show_error_box(std::string function_name);


class SettingsDeleteButtonWidget :  public QWidget, public ConfigWidget, public SettingsResourceDownloadRow::Listener{
public:
    using ParentOption = SettingsResourceDeleteButton;

public:
    ~SettingsDeleteButtonWidget();
    SettingsDeleteButtonWidget(QWidget& parent, SettingsResourceDeleteButton& value);

    virtual void on_action_state_updated() override;

private:
    void update_UI_state();
    void show_delete_confirm_box();

private:
    SettingsResourceDeleteButton& m_value;
    SettingsResourceDownloadRow& m_row;
    QPushButton* m_button;    
};

class SettingsCancelButtonWidget :  public QWidget, public ConfigWidget, public SettingsResourceDownloadRow::Listener{
public:
    using ParentOption = SettingsResourceCancelButton;

public:
    ~SettingsCancelButtonWidget();
    SettingsCancelButtonWidget(QWidget& parent, SettingsResourceCancelButton& value);

    virtual void on_action_state_updated() override;

private:
    void update_UI_state();
    void show_cancel_confirm_box();

private:
    SettingsResourceCancelButton& m_value;
    SettingsResourceDownloadRow& m_row;
    QPushButton* m_button;    
};

class SettingsProgressBarWidget : public QWidget, public ConfigWidget, public SettingsResourceDownloadRow::Listener{
public:
    using ParentOption = SettingsResourceProgressBar;

public:
    ~SettingsProgressBarWidget();
    SettingsProgressBarWidget(QWidget& parent, SettingsResourceProgressBar& value);

    virtual void on_download_progress(uint64_t bytes_done, uint64_t total_bytes) override;
    virtual void on_unzip_progress(uint64_t bytes_done, uint64_t total_bytes) override;
    virtual void on_hash_progress(uint64_t bytes_done, uint64_t total_bytes) override;

    virtual void on_action_state_updated() override;

private:    
    void update_UI_state();
    void update_progress_bar(int percentage, const std::string& text);
    void update_progress_bar(uint64_t bytes_done, uint64_t total_bytes, const std::string& text);
    
private:
    // SettingsResourceProgressBar& m_value;
    SettingsResourceDownloadRow& m_row;
    QLabel* m_status_label;
    QProgressBar* m_progress_bar;
};

class SettingsDownloadErrorWidget : public QWidget, public ConfigWidget, public GlobalResourceDownloadManager::Listener{
public:
    using ParentOption = SettingsDownloadError;

public:
    ~SettingsDownloadErrorWidget();
    SettingsDownloadErrorWidget(QWidget& parent, SettingsDownloadError& value);

public: // GlobalResourceDownloadManager::Listener
    // virtual void on_all_downloads_finished(){}
    virtual void on_download_failed(const std::string& resource_slug) override;
    virtual void on_exception_caught(const std::string& error_msg) override;

private:
    // show popup for download failed. Will only show 1 pop-up at a time.
    void show_download_failed_box(const std::string& resource_slug);

    // show popup for download error. Will only show 1 pop-up at a time.
    void show_error_box(std::string error_msg);

private:
    std::atomic<bool> m_popup_is_open{false};

};





}
#endif
