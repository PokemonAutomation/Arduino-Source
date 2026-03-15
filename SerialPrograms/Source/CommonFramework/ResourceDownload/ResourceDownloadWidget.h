/*  Resource Download Widget
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ResourceDownloadWidget_H
#define PokemonAutomation_ResourceDownloadWidget_H

#include "Common/Qt/Options/ConfigWidget.h"
#include "ResourceDownloadTable.h"

namespace PokemonAutomation{


class DownloadButtonWidget : public ConfigWidget{
public:
    using ParentOption = ResourceDownloadButton;

public:
    DownloadButtonWidget(QWidget& parent, ResourceDownloadButton& value);
};



}
#endif
