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

class DownloadButtonWidget : public QWidget, public ConfigWidget{
    Q_OBJECT
public:
    using ParentOption = ResourceDownloadButton;

public:
    ~DownloadButtonWidget();
    DownloadButtonWidget(QWidget& parent, ResourceDownloadButton& value);

private:
    ResourceDownloadButton& m_value;
    QPushButton* m_button;

    void update_enabled_status();
    void show_download_confirm_box(
        const std::string& title,
        const std::string& message_body
    );
    
};

void show_error_box(std::string function_name);


class DeleteButtonWidget :  public QWidget, public ConfigWidget{
public:
    using ParentOption = ResourceDeleteButton;

public:
    DeleteButtonWidget(QWidget& parent, ResourceDeleteButton& value);
};

class ProgressBarWidget : public QWidget, public ConfigWidget{
public:
    using ParentOption = ResourceProgressBar;

public:
    ~ProgressBarWidget();
    ProgressBarWidget(QWidget& parent, ResourceProgressBar& value);

private:
    ResourceProgressBar& m_value;
    QLabel* m_status_label;
    QProgressBar* m_progress_bar;
};




}
#endif
