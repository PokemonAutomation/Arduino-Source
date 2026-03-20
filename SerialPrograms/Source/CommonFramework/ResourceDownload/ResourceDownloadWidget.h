/*  Resource Download Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ResourceDownloadWidget_H
#define PokemonAutomation_ResourceDownloadWidget_H

#include <QWidget>
#include "Common/Qt/Options/ConfigWidget.h"
#include "ResourceDownloadTable.h"

namespace PokemonAutomation{

class ResourceDownloadButton;

class DownloadButtonWidget : public QWidget, public ConfigWidget{
public:
    using ParentOption = ResourceDownloadButton;

public:
    ~DownloadButtonWidget();
    DownloadButtonWidget(QWidget& parent, ResourceDownloadButton& value);

private:
    ResourceDownloadButton& m_value;
    
};


class DeleteButtonWidget : public ConfigWidget{
public:
    using ParentOption = ResourceDeleteButton;

public:
    DeleteButtonWidget(QWidget& parent, ResourceDeleteButton& value);
};



}
#endif
