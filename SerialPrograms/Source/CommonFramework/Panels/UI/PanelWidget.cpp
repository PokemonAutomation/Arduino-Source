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
#include "CommonFramework/Panels/PanelInstance.h"
#include "PanelElements.h"
#include "PanelWidget.h"

namespace PokemonAutomation{



QWidget* PanelInstance::make_widget(QWidget& parent, PanelHolder& holder){
    return new PanelWidget(parent, *this, holder);
}


PanelWidget::PanelWidget(
    QWidget& parent,
    PanelInstance& instance,
    PanelHolder& holder
)
    : QWidget(&parent)
    , m_instance(instance)
    , m_holder(holder)
{}

CollapsibleGroupBox* PanelWidget::make_header(QWidget& parent){
    return make_panel_header(
        *this,
        m_instance.descriptor().display_name(),
        m_instance.descriptor().doc_link(),
        m_instance.descriptor().description()
    );
}



}
