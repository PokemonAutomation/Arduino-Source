/*  Resource Download Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ResourceDownloadWidget_H
#define PokemonAutomation_ResourceDownloadWidget_H

#include <QWidget>
#include "Common/Qt/Options/ConfigWidget.h"
// #include "ResourceDownloadTable.h"
#include "ResourceDownloadRow.h"

namespace PokemonAutomation{

// class ResourceDownloadButton;

class DownloadButtonWidget : public QWidget, public ConfigWidget{
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


class DeleteButtonWidget : public ConfigWidget{
public:
    using ParentOption = ResourceDeleteButton;

public:
    DeleteButtonWidget(QWidget& parent, ResourceDeleteButton& value);
};




}
#endif
