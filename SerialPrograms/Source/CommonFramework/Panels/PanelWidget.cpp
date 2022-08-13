/*  Panel
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include "Common/Qt/CollapsibleGroupBox.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/Options/Environment/ThemeSelectorOption.h"
#include "PanelDescriptor.h"
#include "PanelInstance.h"
#include "PanelElements.h"
#include "PanelWidget.h"

namespace PokemonAutomation{


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
