/*  Panel
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include "Common/Qt/CollapsibleGroupBox.h"
#include "CommonFramework/Panels/PanelDescriptor.h"
#include "CommonFramework/Panels/PanelSession.h"
#include "PanelElements.h"
#include "PanelWidget.h"

namespace PokemonAutomation{



QWidget* PanelSession::make_widget(QWidget& parent){
    return new PanelWidget(parent, *this);
}


PanelWidget::PanelWidget(
    QWidget& parent,
    PanelSession& instance
)
    : QWidget(&parent)
    , m_instance(instance)
{}

CollapsibleGroupBox* PanelWidget::make_header(){
    return make_panel_header(
        *this,
        m_instance.descriptor().display_name(),
        m_instance.descriptor().doc_link(),
        m_instance.descriptor().description()
    );
}



}
