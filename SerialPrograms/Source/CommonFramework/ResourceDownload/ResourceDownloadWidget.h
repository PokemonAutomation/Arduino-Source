/*  Resource Download Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ResourceDownloadWidget_H
#define PokemonAutomation_ResourceDownloadWidget_H

#include <QObject>
#include <QLabel>
#include <QProgressBar>
#include "Common/Qt/Options/ConfigWidget.h"
// #include "ResourceDownloadTable.h"
#include "ResourceDownloadRow.h"

namespace PokemonAutomation{

// class ResourceDownloadButton;

class DownloadButtonWidget : public QWidget, public ConfigWidget, public ResourceDownloadRow::DownloadListener{
    Q_OBJECT
public:
    using ParentOption = ResourceDownloadButton;

public:
    ~DownloadButtonWidget();
    DownloadButtonWidget(QWidget& parent, ResourceDownloadButton& value);

    virtual void on_metadata_fetch_finished(std::string popup_message) override;
    virtual void on_exception_caught(std::string function_name) override;
    virtual void on_download_failed() override;
    virtual void on_button_state_updated() override;

private:
    void update_UI_state();
    void show_download_confirm_box(
        const std::string& title,
        const std::string& message_body
    );

private:
    ResourceDownloadButton& m_value;
    QPushButton* m_button;
    
};

void show_error_box(std::string function_name);


class DeleteButtonWidget :  public QWidget, public ConfigWidget, public ResourceDownloadRow::DownloadListener{
public:
    using ParentOption = ResourceDeleteButton;

public:
    DeleteButtonWidget(QWidget& parent, ResourceDeleteButton& value);

    virtual void on_button_state_updated() override;

private:
    void update_UI_state();
    void show_delete_confirm_box();

private:
    ResourceDeleteButton& m_value;
    QPushButton* m_button;    
};

class CancelButtonWidget :  public QWidget, public ConfigWidget, public ResourceDownloadRow::DownloadListener{
public:
    using ParentOption = ResourceCancelButton;

public:
    CancelButtonWidget(QWidget& parent, ResourceCancelButton& value);

    virtual void on_button_state_updated() override;

private:
    void update_UI_state();

private:
    ResourceCancelButton& m_value;
    QPushButton* m_button;    
};

class ProgressBarWidget : public QWidget, public ConfigWidget, public ResourceDownloadRow::DownloadListener{
public:
    using ParentOption = ResourceProgressBar;

public:
    ~ProgressBarWidget();
    ProgressBarWidget(QWidget& parent, ResourceProgressBar& value);

    virtual void on_download_progress(int percentage) override;
    virtual void on_unzip_progress(int percentage) override;
    virtual void on_hash_progress(int percentage) override;

    virtual void on_button_state_updated() override;

private:
    void update_UI_state();
    
private:
    ResourceProgressBar& m_value;
    QLabel* m_status_label;
    QProgressBar* m_progress_bar;
};




}
#endif
