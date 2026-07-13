/*  Resource Download Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_SettingsResourceDownloadWidget_H
#define PokemonAutomation_SettingsResourceDownloadWidget_H

#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include "Common/Qt/Options/ConfigWidget.h"
#include "GlobalResourceDownloadManager.h"
#include "SettingsResourceDownloadRow.h"

namespace PokemonAutomation{


class SettingsDownloadButtonWidget : public QWidget, public ConfigWidget, public SettingsResourceButton::Listener{
public:
    using ParentOption = SettingsResourceDownloadButton;

public:
    ~SettingsDownloadButtonWidget();
    SettingsDownloadButtonWidget(QWidget& parent, SettingsResourceDownloadButton& value);


public: // SettingsResourceButton::Listener
    virtual void on_change_text(const std::string& text) override;


private:
    SettingsResourceDownloadButton& m_value;
    SettingsResourceDownloadRow& m_row;
    QPushButton* m_button;
    
};


class SettingsDeleteButtonWidget :  public QWidget, public ConfigWidget, SettingsResourceButton::Listener{
public:
    using ParentOption = SettingsResourceDeleteButton;

public:
    ~SettingsDeleteButtonWidget();
    SettingsDeleteButtonWidget(QWidget& parent, SettingsResourceDeleteButton& value);


public: // SettingsResourceButton::Listener
    virtual void on_change_text(const std::string& text) override;

private:
    void show_delete_confirm_box();

private:
    SettingsResourceDeleteButton& m_value;
    SettingsResourceDownloadRow& m_row;
    QPushButton* m_button;
};

class SettingsCancelButtonWidget :  public QWidget, public ConfigWidget, SettingsResourceButton::Listener{
public:
    using ParentOption = SettingsResourceCancelButton;

public:
    ~SettingsCancelButtonWidget();
    SettingsCancelButtonWidget(QWidget& parent, SettingsResourceCancelButton& value);

public: // SettingsResourceButton::Listener
    virtual void on_change_text(const std::string& text) override;

private:
    void show_cancel_confirm_box();

private:
    SettingsResourceCancelButton& m_value;
    SettingsResourceDownloadRow& m_row;
    QPushButton* m_button;
};

class SettingsProgressBarWidget : public QWidget, public ConfigWidget, public SettingsResourceProgressBar::Listener{
public:
    using ParentOption = SettingsResourceProgressBar;

public:
    ~SettingsProgressBarWidget();
    SettingsProgressBarWidget(QWidget& parent, SettingsResourceProgressBar& value);

    // Because this widget is within a table, QT doesn't respect setVisible
    // so we need to override update_visibility and manually hide/show the progress_bar.
    virtual void update_visibility() override;

public: // SettingsResourceProgressBar::Listener
    virtual void on_change_text(const std::string& text) override;
    virtual void on_update_progress(uint64_t bytes_done, uint64_t total_bytes) override;
    virtual void on_reset_progress() override;

private:    
    void update_progress_bar(int percentage);
    void update_progress_bar(uint64_t bytes_done, uint64_t total_bytes);
    
private:
    SettingsResourceProgressBar& m_value;
    // SettingsResourceDownloadRow& m_row;
    QLabel* m_status_label;
    QProgressBar* m_progress_bar;
};


// NOTE: This does not inherit ConfigWidget, so it doesn't need to be added to StaticRegistrationQt
class SettingsDownloadPopupWidget : public QObject, public SettingsResourceDownloadRow::Listener{
public:
    ~SettingsDownloadPopupWidget();
    SettingsDownloadPopupWidget(SettingsResourceDownloadRow& row);

public: // SettingsResourceDownloadRow::Listener
    virtual void on_metadata_fetch_finished(const std::string& popup_message) override;

private:
    void show_download_confirm_box(
        const std::string& title,
        const std::string& message_body
    );

private:
    // SettingsDownloadPopup& m_value;
    SettingsResourceDownloadRow& m_row;
    
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
